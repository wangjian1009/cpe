#include <sstream>
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ParseTest.hpp"

TEST_F(ParseTest, struct_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='a1' type='int16'/>"
        "    </struct>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='m_s' type='S'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

#pragma pack(push,1)
    struct {
        struct {
            int16_t a1;
        } m_s;
        int16_t a2;
    } expect = { { 12 }, 14  };
#pragma pack(pop)

    ASSERT_EQ(metaSize("S2"), read("<Data><m_s><a1>12</a1></m_s><a2>14</a2></Data>", "S2"));

    ASSERT_XML_READ_RESULT(expect);
}

TEST_F(ParseTest, struct_ignore_unknown_key) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='a1' type='int16'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>"
        );


    ASSERT_EQ(metaSize("S2"), read("<Data><a1>12</a1><not-exist>15</not-exist><a2>14</a2></Data>", "S2"));

    ASSERT_TRUE(result());

    EXPECT_EQ(12, dr_ctype_read_int16(result(0), CPE_DR_TYPE_INT16));
    EXPECT_EQ(14, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
}

TEST_F(ParseTest, struct_ignore_unknown_key_with_nest) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='a1' type='int16'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

#pragma pack(push,1)
    struct {
        int16_t a1;
        int16_t a2;
    } expect = { 12, 14  };
#pragma pack(pop)

    t_em_set_print();
    ASSERT_EQ(
        metaSize("S2")
        , read("<Data><a1>12</a1>"
               "<not-exist><a2>15</a2></not-exist>"
               "<a2>14</a2></Data>", "S2"));

    ASSERT_XML_READ_RESULT(expect);
}

TEST_F(ParseTest, struct_ignore_nest_not_struct) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='a1' type='int16'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

    ASSERT_EQ(metaSize("S2")
              , read("<Data><a1><a2>15</a2></a1>"
                     "<a2>14</a2></Data>", "S2"));

    ASSERT_TRUE(result());

    EXPECT_EQ(14, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
}

TEST_F(ParseTest, struct_ignore_nest_level_2) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='a1' type='int16'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

    ASSERT_EQ(
        metaSize("S2")
        , read("<Data><a1><a2><a2>15</a2></a2></a1>"
               "<a2>14</a2></Data>", "S2"));

    EXPECT_EQ(14, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
}

TEST_F(ParseTest, struct_ignore_overflow_level) {
    ::std::ostringstream metaS;
    metaS << 
        "<metalib tagsetversion='1' name='net'  version='1'>\n"
        "    <struct name='L1' version='1'>\n"
        "	     <entry name='a1' type='int16'/>\n"
        "    </struct>\n";
    for(int i = 0; i < (CPE_DR_MAX_LEVEL - 1); ++i) {
        metaS <<
            "    <struct name='L" << (i + 2) << "' version='1'>\n"
            "	     <entry name='a" << (i + 2) << "' type='L" << (i + 1) << "'/>\n"
            "    </struct>\n";
    }
    metaS <<
        "    <struct name='L33' version='1'>\n"
        "	     <entry name='a33' type='L32'/>\n"
        "	     <entry name='b1' type='int16'/>\n"
        "    </struct>"
        "</metalib>";

    installMeta(metaS.str().c_str());

    ::std::ostringstream dataS;
    dataS << "<Data>";
    for(int i = CPE_DR_MAX_LEVEL + 1; i > 0; i--) {
        dataS << "<a" << i << ">";
    }

    dataS << "23";

    for(int i = 0; i < CPE_DR_MAX_LEVEL + 1; i++) {
        dataS << "</a" << (i + 1) << ">";
    }

    dataS << "<b1>14</b1></Data>";

    t_em_set_print();

    ASSERT_EQ(dr_code_error_internal, read(dataS.str().c_str(), "L33"));

    EXPECT_EQ(14, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
}
