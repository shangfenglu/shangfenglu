#include <stdio.h>
#include <stdint.h>

int test = 1;

// 应用返回的错误码
enum EnumErrorCode {
    ERRCODE_SUCCESS = 0,                    // 成功
    ERRCODE_NOT_SUPPORT_METHOD = 10000,     // 不支持的方法，比如错误的接口传输，没有权限访问的接口
    ERRCODE_PARAM_EMPTY = 10001,            // 没有任何参数
    ERRCODE_UNDEFINED = -1                  // 未定义
};

// 错误码结构
typedef struct ErrorCode {
    int errcode;         // 错误码
    unsigned int appcode; // 应用上的返回码
    const char* edesc;   // 英文描述
    const char* cdesc;   // 中文描述
} ErrorCode;

// 错误码数组
static ErrorCode m_errcode[] = {
    {ERRCODE_SUCCESS, 0, "Success", "成功"},
    {ERRCODE_NOT_SUPPORT_METHOD, 10000, "Not supported method", "不支持的方法，比如错误的接口传输，没有权限访问的接口"},
    {ERRCODE_PARAM_EMPTY, 10001, "Parameter is empty", "没有任何参数"},
    {ERRCODE_UNDEFINED, (unsigned int)-1, "Undefined", "未定义"}
};

// Function prototypes
static int32_t ErrorCode_switchToApp(int32_t code);
const char* ErrorCode_getDescription(int32_t* code);
const char* ErrorCode_getEngDescription(int32_t* code);
const char* ErrorCode_getChnDescription(int32_t* code);

// Function implementations
static int32_t ErrorCode_switchToApp(int32_t code) {
    int32_t rc;
    switch (code) {
        case ERRCODE_UNDEFINED:
            rc = ERRCODE_UNDEFINED; // You can change this as needed
            break;
        default:
            rc = code;
            break;
    }

    return rc;
}

const char* ErrorCode_getDescription(int32_t* code) {
    if (test == 1) {
        return ErrorCode_getChnDescription(code);
    }else{
        return ErrorCode_getEngDescription(code);
    }
}

const char* ErrorCode_getEngDescription(int32_t* code) {
    int32_t s, e, m, n;
    uint32_t rc;

    if (*code == ERRCODE_SUCCESS) {
        n = 0;
        goto END;
    }
    rc = ErrorCode_switchToApp(*code);

    s = 0;
    n = sizeof(m_errcode) / sizeof(ErrorCode) - 1;
    e = n;
    while (s <= e) {
        m = (s + e) >> 1;
        if (m_errcode[m].errcode == (int)rc) { // Cast rc to int for comparison
            n = m;
            break;
        } else if (m_errcode[m].errcode > (int)rc) { // Cast rc to int for comparison
            e = m - 1;
        } else {
            s = m + 1;
        }
    }
END:
    *code = m_errcode[n].appcode;

    return m_errcode[n].edesc;
}

const char* ErrorCode_getChnDescription(int32_t* code) {
    int32_t s, e, m, n;
    uint32_t rc;

    if (*code == ERRCODE_SUCCESS) {
        n = 0;
        goto END;
    }
    rc = ErrorCode_switchToApp(*code);

    s = 0;
    n = sizeof(m_errcode) / sizeof(ErrorCode) - 1;
    e = n;
    while (s <= e) {
        m = (s + e) >> 1;
        if (m_errcode[m].errcode == (int)rc) { // Cast rc to int for comparison
            n = m;
            break;
        } else if (m_errcode[m].errcode > (int)rc) { // Cast rc to int for comparison
            e = m - 1;
        } else {
            s = m + 1;
        }
    }
END:
    *code = m_errcode[n].appcode;

    return m_errcode[n].cdesc;
}

/* int main() {
    int32_t code = ERRCODE_NOT_SUPPORT_METHOD;

    printf("English Description: %s\n", ErrorCode_getEngDescription(&code));
    printf("Chinese Description: %s\n", ErrorCode_getChnDescription(&code));
    printf("App Description: %s\n", ErrorCode_getDescription(&code));

    return 0;
}
 */