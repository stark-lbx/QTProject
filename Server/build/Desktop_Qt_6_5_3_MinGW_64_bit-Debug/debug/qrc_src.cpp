/****************************************************************************
** Resource object code
**
** Created by: The Resource Compiler for Qt version 6.5.3
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

static const unsigned char qt_resource_data[] = {
  // D:/Coding/QtProject/Pointer/Server/server.config
  0x0,0x0,0x0,0x1d,
  0x31,
  0x32,0x37,0x2e,0x30,0x2e,0x30,0x2e,0x31,0xd,0xa,0x35,0x30,0x30,0x30,0xd,0xa,
  0x2e,0x2f,0x73,0x79,0x73,0x66,0x69,0x6c,0x65,0x73,0xd,0xa,
  
};

static const unsigned char qt_resource_name[] = {
  // cfg
  0x0,0x3,
  0x0,0x0,0x69,0xc7,
  0x0,0x63,
  0x0,0x66,0x0,0x67,
    // server.config
  0x0,0xd,
  0x7,0x39,0xd5,0xb7,
  0x0,0x73,
  0x0,0x65,0x0,0x72,0x0,0x76,0x0,0x65,0x0,0x72,0x0,0x2e,0x0,0x63,0x0,0x6f,0x0,0x6e,0x0,0x66,0x0,0x69,0x0,0x67,
  
};

static const unsigned char qt_resource_struct[] = {
  // :
  0x0,0x0,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x1,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  // :/cfg
  0x0,0x0,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x2,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  // :/cfg/server.config
  0x0,0x0,0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,
0x0,0x0,0x1,0x96,0x80,0xf5,0x15,0x24,

};

#ifdef QT_NAMESPACE
#  define QT_RCC_PREPEND_NAMESPACE(name) ::QT_NAMESPACE::name
#  define QT_RCC_MANGLE_NAMESPACE0(x) x
#  define QT_RCC_MANGLE_NAMESPACE1(a, b) a##_##b
#  define QT_RCC_MANGLE_NAMESPACE2(a, b) QT_RCC_MANGLE_NAMESPACE1(a,b)
#  define QT_RCC_MANGLE_NAMESPACE(name) QT_RCC_MANGLE_NAMESPACE2( \
        QT_RCC_MANGLE_NAMESPACE0(name), QT_RCC_MANGLE_NAMESPACE0(QT_NAMESPACE))
#else
#   define QT_RCC_PREPEND_NAMESPACE(name) name
#   define QT_RCC_MANGLE_NAMESPACE(name) name
#endif

#ifdef QT_NAMESPACE
namespace QT_NAMESPACE {
#endif

bool qRegisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);
bool qUnregisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);

#ifdef QT_NAMESPACE
}
#endif

int QT_RCC_MANGLE_NAMESPACE(qInitResources_src)();
int QT_RCC_MANGLE_NAMESPACE(qInitResources_src)()
{
    int version = 3;
    QT_RCC_PREPEND_NAMESPACE(qRegisterResourceData)
        (version, qt_resource_struct, qt_resource_name, qt_resource_data);
    return 1;
}

int QT_RCC_MANGLE_NAMESPACE(qCleanupResources_src)();
int QT_RCC_MANGLE_NAMESPACE(qCleanupResources_src)()
{
    int version = 3;
    QT_RCC_PREPEND_NAMESPACE(qUnregisterResourceData)
       (version, qt_resource_struct, qt_resource_name, qt_resource_data);
    return 1;
}

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

namespace {
   struct initializer {
       initializer() { QT_RCC_MANGLE_NAMESPACE(qInitResources_src)(); }
       ~initializer() { QT_RCC_MANGLE_NAMESPACE(qCleanupResources_src)(); }
   } dummy;
}

#ifdef __clang__
#   pragma clang diagnostic pop
#endif
