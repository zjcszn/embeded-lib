// AUTOGENERATED FILE. DO NOT EDIT.
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include "cmock.h"
#include "MockLed.h"

static const char* CMockString_Led_off = "Led_off";
static const char* CMockString_Led_on = "Led_on";
static const char* CMockString_index = "index";

typedef struct _CMOCK_Led_on_CALL_INSTANCE
{
  UNITY_LINE_TYPE LineNumber;
  uint32_t ReturnVal;
  uint8_t Expected_index;

} CMOCK_Led_on_CALL_INSTANCE;

typedef struct _CMOCK_Led_off_CALL_INSTANCE
{
  UNITY_LINE_TYPE LineNumber;
  uint8_t Expected_index;

} CMOCK_Led_off_CALL_INSTANCE;

static struct MockLedInstance
{
  CMOCK_MEM_INDEX_TYPE Led_on_CallInstance;
  CMOCK_MEM_INDEX_TYPE Led_off_CallInstance;
} Mock;

extern jmp_buf AbortFrame;

void MockLed_Verify(void)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK_MEM_INDEX_TYPE call_instance;
  call_instance = Mock.Led_on_CallInstance;
  if (CMOCK_GUTS_NONE != call_instance)
  {
    UNITY_SET_DETAIL(CMockString_Led_on);
    UNITY_TEST_FAIL(cmock_line, CMockStringCalledLess);
  }
  call_instance = Mock.Led_off_CallInstance;
  if (CMOCK_GUTS_NONE != call_instance)
  {
    UNITY_SET_DETAIL(CMockString_Led_off);
    UNITY_TEST_FAIL(cmock_line, CMockStringCalledLess);
  }
}

void MockLed_Init(void)
{
  MockLed_Destroy();
}

void MockLed_Destroy(void)
{
  CMock_Guts_MemFreeAll();
  memset(&Mock, 0, sizeof(Mock));
}

uint32_t Led_on(uint8_t index)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK_Led_on_CALL_INSTANCE* cmock_call_instance;
  UNITY_SET_DETAIL(CMockString_Led_on);
  cmock_call_instance = (CMOCK_Led_on_CALL_INSTANCE*)CMock_Guts_GetAddressFor(Mock.Led_on_CallInstance);
  Mock.Led_on_CallInstance = CMock_Guts_MemNext(Mock.Led_on_CallInstance);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringCalledMore);
  cmock_line = cmock_call_instance->LineNumber;
  {
    UNITY_SET_DETAILS(CMockString_Led_on,CMockString_index);
    UNITY_TEST_ASSERT_EQUAL_HEX8(cmock_call_instance->Expected_index, index, cmock_line, CMockStringMismatch);
  }
  UNITY_CLR_DETAILS();
  return cmock_call_instance->ReturnVal;
}

void CMockExpectParameters_Led_on(CMOCK_Led_on_CALL_INSTANCE* cmock_call_instance, uint8_t index);
void CMockExpectParameters_Led_on(CMOCK_Led_on_CALL_INSTANCE* cmock_call_instance, uint8_t index)
{
  cmock_call_instance->Expected_index = index;
}

void Led_on_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, uint8_t index, uint32_t cmock_to_return)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK_Led_on_CALL_INSTANCE));
  CMOCK_Led_on_CALL_INSTANCE* cmock_call_instance = (CMOCK_Led_on_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.Led_on_CallInstance = CMock_Guts_MemChain(Mock.Led_on_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  CMockExpectParameters_Led_on(cmock_call_instance, index);
  cmock_call_instance->ReturnVal = cmock_to_return;
}

void Led_off(uint8_t index)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK_Led_off_CALL_INSTANCE* cmock_call_instance;
  UNITY_SET_DETAIL(CMockString_Led_off);
  cmock_call_instance = (CMOCK_Led_off_CALL_INSTANCE*)CMock_Guts_GetAddressFor(Mock.Led_off_CallInstance);
  Mock.Led_off_CallInstance = CMock_Guts_MemNext(Mock.Led_off_CallInstance);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringCalledMore);
  cmock_line = cmock_call_instance->LineNumber;
  {
    UNITY_SET_DETAILS(CMockString_Led_off,CMockString_index);
    UNITY_TEST_ASSERT_EQUAL_HEX8(cmock_call_instance->Expected_index, index, cmock_line, CMockStringMismatch);
  }
  UNITY_CLR_DETAILS();
}

void CMockExpectParameters_Led_off(CMOCK_Led_off_CALL_INSTANCE* cmock_call_instance, uint8_t index);
void CMockExpectParameters_Led_off(CMOCK_Led_off_CALL_INSTANCE* cmock_call_instance, uint8_t index)
{
  cmock_call_instance->Expected_index = index;
}

void Led_off_CMockExpect(UNITY_LINE_TYPE cmock_line, uint8_t index)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK_Led_off_CALL_INSTANCE));
  CMOCK_Led_off_CALL_INSTANCE* cmock_call_instance = (CMOCK_Led_off_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.Led_off_CallInstance = CMock_Guts_MemChain(Mock.Led_off_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  CMockExpectParameters_Led_off(cmock_call_instance, index);
}

