#ifndef HTTP_SERVER_ENUM_DEF_H
#define HTTP_SERVER_ENUM_DEF_H

#define ENUM_VALUE2(type, val1, val2) ENUM_VALUE3(type, val1, val2, _empty_)
#define ENUM_VALUE3(type, val1, val2, val3) type(val1, val2, val3)

#define ENUM_TYPE_VAL1(val1, val2, val3) val1
#define ENUM_TYPE_VAL2(val1, val2, val3) val2
#define ENUM_TYPE_VAL3(val1, val2, val3) val3

#endif
