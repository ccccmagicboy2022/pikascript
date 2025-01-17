#include "gtest/gtest.h"
extern "C" {
#include "BaseObj.h"
#include "PikaMain.h"
#include "PikaMath_Operator.h"
#include "PikaParser.h"
#include "PikaStdLib_SysObj.h"
#include "PikaVM.h"
#include "dataMemory.h"
#include "dataQueue.h"
#include "dataStrs.h"
}

TEST(VM, num1) {
    char* line = (char*)"1";
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_LineToAsm(buffs, line, NULL);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    pikaVM_runAsm(self, pikaAsm);
    obj_deinit(self);
    args_deinit(buffs);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, a_1) {
    char* line = (char*)"a = 1";
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_LineToAsm(buffs, line, NULL);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    pikaVM_runAsm(self, pikaAsm);
    ASSERT_EQ(args_getInt(self->list, (char*)"a"), 1);

    obj_deinit(self);
    args_deinit(buffs);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, a_1d1) {
    char* line = (char*)"a = 1.1";
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_LineToAsm(buffs, line, NULL);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);
    ASSERT_FLOAT_EQ(args_getFloat(globals->list, (char*)"a"), 1.1);

    obj_deinit(self);
    // obj_deinit(globals);
    args_deinit(buffs);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, str_xy) {
    char* line = (char*)"a = 'xy'";
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_LineToAsm(buffs, line, NULL);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);

    ASSERT_STREQ(args_getStr(globals->list, (char*)"a"), (char*)"xy");

    obj_deinit(self);
    // obj_deinit(globals);
    args_deinit(buffs);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, str_xy_space) {
    char* line = (char*)"a = 'xy '";
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_LineToAsm(buffs, line, NULL);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);

    ASSERT_STREQ(args_getStr(globals->list, (char*)"a"), (char*)"xy ");

    obj_deinit(self);
    args_deinit(buffs);
    // obj_deinit(globals);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, ref_a_b) {
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    Args* buffs = New_strBuff();

    VM_Parameters* globals =
        pikaVM_runAsm(self, Parser_LineToAsm(buffs, (char*)"a = 'xy '", NULL));
    globals = pikaVM_runAsmWithPars(
        self, globals, globals, Parser_LineToAsm(buffs, (char*)"b = a", NULL));

    args_deinit(buffs);
    ASSERT_STREQ(args_getStr(globals->list, (char*)"b"), (char*)"xy ");
    obj_deinit(self);
    // obj_deinit(globals);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, Run_add) {
    PikaObj* self = newRootObj((char*)"root", New_PikaMath_Operator);
    Args* buffs = New_strBuff();

    VM_Parameters* globals = pikaVM_runAsm(
        self, Parser_LineToAsm(buffs, (char*)"a = plusInt(1,2)", NULL));

    args_deinit(buffs);
    int a = args_getInt(globals->list, (char*)"a");
    ASSERT_EQ(a, 3);
    obj_deinit(self);
    // obj_deinit(globals);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, Run_add_multy) {
    PikaObj* self = newRootObj((char*)"root", New_PikaMath_Operator);
    Args* buffs = New_strBuff();

    VM_Parameters* globals =
        pikaVM_runAsm(self, Parser_LineToAsm(buffs, (char*)"b = 2", NULL));
    globals = pikaVM_runAsmWithPars(
        self, globals, globals,
        Parser_LineToAsm(buffs, (char*)"a = plusInt(1,b)", NULL));

    args_deinit(buffs);
    int a = args_getInt(globals->list, (char*)"a");
    ASSERT_EQ(a, 3);
    obj_deinit(self);
    // obj_deinit(globals);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, Run_add_1_2_3) {
    PikaObj* self = newRootObj((char*)"root", New_PikaMath_Operator);
    Args* buffs = New_strBuff();

    VM_Parameters* globals = pikaVM_runAsm(
        self,
        Parser_LineToAsm(buffs, (char*)"a = plusInt(1, plusInt(2,3) )", NULL));

    args_deinit(buffs);
    int a = args_getInt(globals->list, (char*)"a");
    ASSERT_EQ(a, 6);
    obj_deinit(self);
    // obj_deinit(globals);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, JEZ) {
    char* pikaAsm = (char*)
    "B0\n"
    "0 REF False\n"
    "0 JEZ 2\n"
    "B0\n"
    "B0\n";
    PikaObj* self = New_TinyObj(NULL);
    int lineAddr = 0;
    Args* sysRes = New_args(NULL);
    args_setErrorCode(sysRes, 0);
    args_setSysOut(sysRes, (char*)"");
    VM_Parameters* globals = self;
    lineAddr = pikaVM_runAsmLine(self, globals, globals, pikaAsm, lineAddr);
    lineAddr = pikaVM_runAsmLine(self, globals, globals, pikaAsm, lineAddr);
    lineAddr = pikaVM_runAsmLine(self, globals, globals, pikaAsm, lineAddr);
    __clearInvokeQueues(globals);
    obj_deinit(self);
    args_deinit(sysRes);
    // obj_deinit(globals);
    EXPECT_EQ(lineAddr, 26);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, JMP) {
    char* pikaAsm = (char*)
    "B0\n"
    "0 JMP 2\n"
    "B0\n"
    "B0\n";
    PikaObj* self = New_TinyObj(NULL);
    int lineAddr = 0;
    Args* sysRes = New_args(NULL);
    args_setErrorCode(sysRes, 0);
    args_setSysOut(sysRes, (char*)"");
    VM_Parameters* globals = self;
    lineAddr = pikaVM_runAsmLine(self, globals, globals, pikaAsm, lineAddr);
    lineAddr = pikaVM_runAsmLine(self, globals, globals, pikaAsm, lineAddr);
    __clearInvokeQueues(globals);
    obj_deinit(self);
    args_deinit(sysRes);
    // obj_deinit(globals);
    EXPECT_EQ(lineAddr, 14);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, JMP_back1) {
    char* pikaAsm = (char*)
    "B0\n"
    "B0\n"
    "0 JMP -1\n"
    "B0\n"
    "B0\n";
    PikaObj* self = New_TinyObj(NULL);
    int lineAddr = 0;
    Args* sysRes = New_args(NULL);
    args_setErrorCode(sysRes, 0);
    args_setSysOut(sysRes, (char*)"");
    VM_Parameters* globals = self;
    lineAddr = pikaVM_runAsmLine(self, globals, globals, pikaAsm, lineAddr);
    lineAddr = pikaVM_runAsmLine(self, globals, globals, pikaAsm, lineAddr);
    lineAddr = pikaVM_runAsmLine(self, globals, globals, pikaAsm, lineAddr);
    __clearInvokeQueues(globals);
    obj_deinit(self);
    args_deinit(sysRes);
    // obj_deinit(globals);
    EXPECT_EQ(lineAddr, 0);
    EXPECT_EQ(pikaMemNow(), 0);
}

extern PikaMemInfo pikaMemInfo;
TEST(VM, WHILE) {
    pikaMemInfo.heapUsedMax = 0;
    Args* buffs = New_strBuff();
    char* lines =(char *)
        "a = 1\n"
        "b = 0\n"
        "while a:\n"
        "    b = 1\n"
        "    a = 0\n"
        "\n";
    printf("%s", lines);
    char* pikaAsm = Parser_multiLineToAsm(buffs, (char*)lines);
    printf("%s", pikaAsm);
    pikaMemInfo.heapUsedMax = 0;
    PikaObj* self = New_TinyObj(NULL);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);
    EXPECT_EQ(args_getInt(globals->list, (char*)"a"), 0);
    EXPECT_EQ(args_getInt(globals->list, (char*)"b"), 1);
    // obj_deinit(globals);
    args_deinit(buffs);
    obj_deinit(self);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, a_1_1) {
    char* line = (char*)"a = 1 + 1";
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_LineToAsm(buffs, line, NULL);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);

    int res = args_getInt(globals->list, (char*)"a");
    obj_deinit(self);
    // obj_deinit(globals);
    args_deinit(buffs);
    ASSERT_EQ(res, 2);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, a_1_1d1) {
    char* line = (char*)"a = 1 + 1.1";
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_LineToAsm(buffs, line, NULL);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);

    float res = args_getFloat(globals->list, (char*)"a");
    obj_deinit(self);
    // obj_deinit(globals);
    args_deinit(buffs);
    ASSERT_FLOAT_EQ(res, 2.1);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, a_jjcc) {
    char* line = (char*)"a = (1 + 1.1) * 3 - 2 /4.0";
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_LineToAsm(buffs, line, NULL);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);

    float res = args_getFloat(globals->list, (char*)"a");
    obj_deinit(self);
    args_deinit(buffs);
    // obj_deinit(globals);
    ASSERT_FLOAT_EQ(res, 5.8);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, while_a_1to10) {
    char* line = (char*)
    "a = 0\n"
    "while a < 10:\n"
    "    a = a + 1\n"
    "    print(a)\n"
    "\n";
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_multiLineToAsm(buffs, line);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);

    int res = args_getInt(globals->list, (char*)"a");
    obj_deinit(self);
    args_deinit(buffs);
    // obj_deinit(globals);
    ASSERT_FLOAT_EQ(res, 10);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, mem_x) {
    char* line = (char*)
    "mem = PikaStdLib.MemChecker()\n"
    "mem.x = 1\n"
    "print(mem.x)\n"
    "mem.x = 2\n"
    "print(mem.x)\n"
    "\n";
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_multiLineToAsm(buffs, line);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"", New_PikaMain);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);

    int res = obj_getInt(globals, (char*)"mem.x");
    EXPECT_EQ(2, res);
    obj_deinit(self);
    // obj_deinit(globals);
    args_deinit(buffs);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, DEF_instruct) {
    char* pikaAsm = (char*)
    "B0\n"
    "0 DEF test()\n"
    "0 JMP 1\n"
    "B1\n"
    "0 NUM 1\n"
    "0 RET\n"
    "B0\n";
    char* methodCode = (char*)
    "B1\n"
    "0 NUM 1\n"
    "0 RET\n"
    "B0\n";
    PikaObj* self = New_TinyObj(NULL);
    pikaVM_runAsm(self, pikaAsm);
    char* methodPtr = (char*)obj_getPtr(self, (char*)"test");
    EXPECT_STREQ(methodCode, methodPtr);
    obj_deinit(self);
    // obj_deinit(globals);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, RET_instruct) {
    char* pikaAsm = (char*)
    "B1\n"
    "0 NUM 13\n"
    "0 RET\n"
    "0 NUM 2\n"
    "0 RET\n"
    "B0\n";
    PikaObj* self = New_TinyObj(NULL);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);
    Arg* returnArg = args_getArg(globals->list, (char*)"return");
    int num = arg_getInt(returnArg);
    EXPECT_EQ(num, 13);
    obj_deinit(self);
    // obj_deinit(globals);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, RUN_DEF) {
    char* pikaAsm = (char*)
    "B0\n"
    "0 DEF test()\n"
    "0 JMP 1\n"
    "B1\n"
    "0 NUM 1\n"
    "0 RET\n"
    "B0\n"
    "0 RUN test\n"
    "0 OUT a\n";
    PikaObj* self = New_TinyObj(NULL);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);
    int num = obj_getInt(globals, (char*)"a");
    EXPECT_EQ(num, 1);
    obj_deinit(self);
    // obj_deinit(globals);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, RUN_global) {
    char* pikaAsm = (char*)
    "B0\n"
    "0 NUM 1\n"
    "0 OUT a\n"
    "B0\n"
    "0 DEF test()\n"
    "0 JMP 1\n"
    "B1\n"
    "0 REF a\n"
    "0 RET\n"
    "B0\n"
    "0 RUN test\n"
    "0 OUT b\n";
    PikaObj* self = New_TinyObj(NULL);
    pikaVM_runAsm(self, pikaAsm);
    int a = obj_getInt(self, (char*)"a");
    int b = obj_getInt(self, (char*)"b");
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 1);
    obj_deinit(self);
    // obj_deinit(globals);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, RUN_local_b) {
    char* pikaAsm = (char*)
    "B0\n"
    "0 NUM 1\n"
    "0 OUT a\n"
    "B0\n"
    "0 DEF test()\n"
    "0 JMP 1\n"
    "B1\n"
    "0 REF a\n"
    "0 OUT b\n"
    "1 REF b\n"
    "1 REF a\n"
    "0 OPT +\n"
    "0 RET\n"
    "B0\n"
    "0 RUN test\n"
    "0 OUT c\n";
    PikaObj* self = newRootObj((char*)"", New_BaseObj);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);
    int a = obj_getInt(globals, (char*)"a");
    int b = obj_getInt(globals, (char*)"b");
    int c = obj_getInt(globals, (char*)"c");
    EXPECT_EQ(a, 1);
    /* b is local, should not be exist in globals */
    EXPECT_EQ(b, -999999999);
    EXPECT_EQ(c, 2);
    obj_deinit(self);
    // obj_deinit(globals);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, RUN_DEF_add) {
    char* pikaAsm = (char*)
    "B0\n"
    "0 DEF add(a,b)\n"
    "0 JMP 1\n"
    "B1\n"
    "1 REF b\n"
    "1 REF a\n"
    "0 OPT +\n"
    "0 RET\n"
    "B0\n"
    "1 NUM 1\n"
    "1 NUM 2\n"
    "0 RUN add\n"
    "0 OUT c\n";
    PikaObj* self = newRootObj((char*)"", New_BaseObj);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);
    int a = obj_getInt(globals, (char*)"a");
    int b = obj_getInt(globals, (char*)"b");
    int c = obj_getInt(globals, (char*)"c");
    /* a is local, should not be exist in globals */
    EXPECT_EQ(a, -999999999);
    /* b is local, should not be exist in globals */
    EXPECT_EQ(b, -999999999);
    EXPECT_EQ(c, 3);
    obj_deinit(self);
    // obj_deinit(globals);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, run_def_add) {
    char* line = (char*)
    "def add(a, b):\n"
    "    return a + b\n"
    "c = add(1,3)\n"
    "\n";
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_multiLineToAsm(buffs, line);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);

    int c = args_getInt(globals->list, (char*)"c");
    obj_deinit(self);
    args_deinit(buffs);
    // obj_deinit(globals);
    ASSERT_FLOAT_EQ(c, 4);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, equ) {
    char* line = (char*)
    "if -1 == -1:\n"
    "    a = 1\n"
    "\n"
    ;
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_multiLineToAsm(buffs, line);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);

    int c = args_getInt(globals->list, (char*)"a");
    obj_deinit(self);
    args_deinit(buffs);
    // obj_deinit(globals);
    ASSERT_FLOAT_EQ(c, 1);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, if_elif) {
    char* line = (char*)
    "a = 2\n"
    "b = 0\n"
    "if a > 1:\n"
    "    b = 1\n"
    "elif a > 0:\n"
    "    b = 2\n"
    "\n"
    ;
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_multiLineToAsm(buffs, line);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);

    int b = args_getInt(globals->list, (char*)"b");
    obj_deinit(self);
    args_deinit(buffs);
    // obj_deinit(globals);
    ASSERT_FLOAT_EQ(b, 1);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, if_else) {
    char* line = (char*)
    "a = 0\n"
    "b = 0\n"
    "if a > 1:\n"
    "    b = 1\n"
    "else:\n"
    "    b = 2\n"
    "\n"
    ;
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_multiLineToAsm(buffs, line);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);

    int b = args_getInt(globals->list, (char*)"b");
    obj_deinit(self);
    args_deinit(buffs);
    // obj_deinit(globals);
    ASSERT_FLOAT_EQ(b, 2);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, if_else_) {
    char* line = (char*)
    "a = 2\n"
    "b = 0\n"
    "if a > 1:\n"
    "    b = 1\n"
    "else:\n"
    "    b = 2\n"
    "\n"
    ;
    Args* buffs = New_strBuff();
    char* pikaAsm = Parser_multiLineToAsm(buffs, line);
    printf("%s", pikaAsm);
    PikaObj* self = newRootObj((char*)"root", New_PikaStdLib_SysObj);
    VM_Parameters* globals = pikaVM_runAsm(self, pikaAsm);

    int b = args_getInt(globals->list, (char*)"b");
    obj_deinit(self);
    args_deinit(buffs);
    // obj_deinit(globals);
    ASSERT_FLOAT_EQ(b, 1);
    EXPECT_EQ(pikaMemNow(), 0);
}

TEST(VM, EST) {
    char* pikaAsm = (char*)
    "B0\n"
    "0 EST a\n"
    "0 OUT b\n"
    ;
    PikaObj* self = newRootObj((char*)"", New_BaseObj);
    pikaVM_runAsm(self, pikaAsm);
    int a = obj_getInt(self, (char*)"a");
    int b = obj_getInt(self, (char*)"b");
    /* a is local, should not be exist in globals */
    EXPECT_EQ(a, -999999999);
    /* b is local, should not be exist in globals */
    EXPECT_EQ(b, 0);
    obj_deinit(self);
    // obj_deinit(globals);
    EXPECT_EQ(pikaMemNow(), 0);
}