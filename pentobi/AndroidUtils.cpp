//-----------------------------------------------------------------------------
/** @file pentobi/AndroidUtils.cpp
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

#include "AndroidUtils.h"

#include <QFileInfo>

#ifdef Q_OS_ANDROID

#include <QBuffer>
#include <QCoreApplication>
#include <QHash>
#include <QImage>
#include <QVariant>
#include <QtCore/private/qandroidextras_p.h>
#include <QJniObject>

#else

#include <QStandardPaths>

#endif

using namespace std;
#ifdef Q_OS_ANDROID
using QNativeInterface::QAndroidApplication;
#endif

//-----------------------------------------------------------------------------

namespace {

#ifdef Q_OS_ANDROID

struct AutoClose
{
    const QJniObject& m_obj;

    ~AutoClose()
    {
        m_obj.callMethod<void>("close");
    }
};

QJniObject getContext()
{
    return {QAndroidApplication::context()};
}

QJniObject getContentResolver()
{
    return getContext().callObjectMethod(
                "getContentResolver", "()Landroid/content/ContentResolver;");
}

QString getDisplayName(const QJniObject& uri)
{
    auto contentResolver = getContentResolver();
    if (! contentResolver.isValid())
        return {};
    QJniEnvironment env;
    auto stringClass = env->FindClass("java/lang/String");
    auto projection = env->NewObjectArray(1, stringClass, nullptr);
    auto column = QJniObject::getStaticObjectField<jstring>(
                "android/provider/OpenableColumns", "DISPLAY_NAME");
    if (! column.isValid())
        return {};
    env->SetObjectArrayElement(projection, 0, column.object());
    auto cursor = contentResolver.callObjectMethod(
                "query",
                "(Landroid/net/Uri;[Ljava/lang/String;Ljava/lang/String;"
                "[Ljava/lang/String;Ljava/lang/String;)"
                "Landroid/database/Cursor;",
                uri.object(), projection, nullptr, nullptr, nullptr);
    if (env->ExceptionCheck())
        return {};
    AutoClose autoClose{cursor};
    if (! cursor.callMethod<jboolean>("moveToFirst"))
        return {};
    auto index = cursor.callMethod<jint>(
                "getColumnIndex", "(Ljava/lang/String;)I", column.object());
    if (index < 0)
        return {};
    auto name = cursor.callObjectMethod(
                "getString", "(I)Ljava/lang/String;", index);
    if (! name.isValid())
        return {};
    return name.toString();
}

QJniObject getUriObj(const QString& uri)
{
    auto uriString = QJniObject::fromString(uri);
    return QJniObject::callStaticObjectMethod(
                "android/net/Uri", "parse",
                "(Ljava/lang/String;)Landroid/net/Uri;",
                uriString.object<jstring>());
}

#endif

} // namespace

//-----------------------------------------------------------------------------

QString AndroidUtils::m_error;

#ifdef Q_OS_ANDROID
bool AndroidUtils::checkException()
{
    QJniEnvironment env;
    if (! env->ExceptionCheck())
        return false;
    auto e = env->ExceptionOccurred();
    env->ExceptionClear();
    auto method = env->GetMethodID(env->GetObjectClass(e),
                                   "getLocalizedMessage",
                                   "()Ljava/lang/String;");
    QJniObject message(env->CallObjectMethod(e, method));
    m_error = message.toString();
    return true;
}
#endif

bool AndroidUtils::checkExists(const QString& file)
{
#ifdef Q_OS_ANDROID
    if (QUrl(file).isRelative())
        return QFileInfo::exists(file);
    // Note: using ContentResolver::query() on persisted URIs without calling
    // ACTION_OPEN_DOCUMENT first only works on some devices. Maybe try
    // DocumentFile.exist(DocumentFile.fromSingleUri()) once we require a
    // Qt version that supports androidx (see also QTBUG-73904).
    auto contentResolver = getContentResolver();
    if (! contentResolver.isValid())
        return false;
    auto uriObj = getUriObj(file);
    if (! uriObj.isValid())
        return false;
    QJniEnvironment env;
    auto stringClass = env->FindClass("java/lang/String");
    auto projection = env->NewObjectArray(1, stringClass, nullptr);
    auto column = QJniObject::getStaticObjectField<jstring>(
                "android/provider/DocumentsContract$Document",
                "COLUMN_DOCUMENT_ID");
    if (! column.isValid())
        return false;
    env->SetObjectArrayElement(projection, 0, column.object());
    auto cursor = contentResolver.callObjectMethod(
                "query",
                "(Landroid/net/Uri;[Ljava/lang/String;Ljava/lang/String;"
                "[Ljava/lang/String;Ljava/lang/String;)"
                "Landroid/database/Cursor;",
                uriObj.object(), projection, nullptr, nullptr, nullptr);
    if (env->ExceptionCheck())
        return false;
    if (! cursor.isValid())
        return false;
    AutoClose autoClose{cursor};
    auto count = cursor.callMethod<jint>("getCount");
    return count > 0;
#else
    return QFileInfo::exists(file);
#endif
}

void AndroidUtils::exit()
{
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>(
                "java/lang/System", "exit", "(I)V", 0);
#endif
}

QString AndroidUtils::getDisplayName([[maybe_unused]]const QString& uri)
{
#ifdef Q_OS_ANDROID
    if (! QUrl(uri).isRelative())
        return ::getDisplayName(getUriObj(uri));
#endif
    return {};
}

#ifdef Q_OS_ANDROID
QString AndroidUtils::getInitialFile()
{
    auto intent =
            getContext().callObjectMethod(
                "getIntent", "()Landroid/content/Intent;");
    if (! intent.isValid())
        return {};
    auto uri = intent.callObjectMethod("getData", "()Landroid/net/Uri;");
    if (! uri.isValid())
        return {};
    auto uriString =
            uri.callObjectMethod("toString", "()Ljava/lang/String;");
    if (! uriString.isValid())
        return {};
    return uriString.toString();
}
#endif

QStringList AndroidUtils::getPersistedUriPermissions()
{
    QStringList result;
#ifdef Q_OS_ANDROID
    auto contentResolver = getContentResolver();
    if (! contentResolver.isValid())
        return result;
    auto permissions = contentResolver.callObjectMethod(
                "getPersistedUriPermissions", "()Ljava/util/List;");
    if (! permissions.isValid())
        return result;
    auto size = permissions.callMethod<jint>("size");
    for (jint i = 0; i < size; ++i)
    {
        auto p = permissions.callObjectMethod(
                    "get", "(I)Ljava/lang/Object;", i);
        if (! p.isValid())
            continue;
        auto uri = p.callObjectMethod("getUri", "()Landroid/net/Uri;");
        if (! uri.isValid())
            continue;
        auto s = uri.callObjectMethod<jstring>("toString");
        if (! s.isValid())
            continue;
        result.append(s.toString());
    }
#endif
    return result;
}

void AndroidUtils::initTheme([[maybe_unused]]QColor colorBackground)
{
#ifdef Q_OS_ANDROID
    QAndroidApplication::runOnAndroidMainThread([=]() {
        auto window = getContext().callObjectMethod("getWindow",
                                                    "()Landroid/view/Window;");
        if (! window.isValid())
            return;
        window.callMethod<void>(
                    "addFlags", "(I)V",
                    0x80000000 /* FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS */);
        window.callMethod<void>("clearFlags", "(I)V",
                                0x04000000 /* FLAG_TRANSLUCENT_STATUS */);
        auto view = window.callObjectMethod("getDecorView",
                                            "()Landroid/view/View;");
        bool isLight = (colorBackground.lightness() > 128);
        if (QAndroidApplication::sdkVersion() < 30)
        {
            int visibility =
                    view.callMethod<int>("getSystemUiVisibility", "()I");
            if (isLight)
                visibility |= 0x00002000; // SYSTEM_UI_FLAG_LIGHT_STATUS_BAR
            else
                visibility &= ~0x00002000;
            if (QAndroidApplication::sdkVersion() >= 26)
            {
                if (isLight)
                    visibility |= 0x00000010; // SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR
                else
                    visibility &= ~0x00000010;
            }
            view.callMethod<void>("setSystemUiVisibility", "(I)V", visibility);
        }
        else // QAndroidApplication::sdkVersion() >= 30
        {
            auto insetsController =
                    view.callObjectMethod(
                        "getWindowInsetsController",
                        "()Landroid/view/WindowInsetsController;");
            if (isLight)
                insetsController.callMethod<void>(
                            "setSystemBarsAppearance", "(II)V",
                            0x00000008 // APPEARANCE_LIGHT_STATUS_BARS
                            | 0x00000010, // APPEARANCE_LIGHT_NAVIGATION_BARS
                            0x00000008 | 0x00000010);
            else
                insetsController.callMethod<void>(
                            "setSystemBarsAppearance", "(II)V",
                            0, 0x00000008 | 0x00000010);
        }
        window.callMethod<void>("setStatusBarColor", "(I)V",
                                colorBackground.rgba());
        // Set navigation bar color only if we can also use
        // SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR (API level 26)
        if (QAndroidApplication::sdkVersion() >= 26)
            window.callMethod<void>("setNavigationBarColor", "(I)V",
                                    colorBackground.rgba());
    });
#endif
}

#ifdef Q_OS_ANDROID
bool AndroidUtils::open(
        [[maybe_unused]]const QString& uri, [[maybe_unused]]QByteArray& sgf)
{
    m_error.clear();
    auto contentResolver = getContentResolver();
    if (! contentResolver.isValid())
        return false;
    auto uriObj = getUriObj(uri);
    if (! uriObj.isValid())
        return false;
    auto inputStream = contentResolver.callObjectMethod(
                "openInputStream",
                "(Landroid/net/Uri;)Ljava/io/InputStream;", uriObj.object());
    if (checkException())
        return false;
    if (! inputStream.isValid())
        return false;
    sgf.clear();
    QJniEnvironment env;
    const int bufSize = 2048;
    auto byteArray = env->NewByteArray(bufSize);
    jbyte buf[bufSize];
    jint n;
    while ((n = inputStream.callMethod<jint>("read", "([B)I", byteArray)) >= 0)
    {
        if (checkException())
        {
            inputStream.callMethod<void>("close");
            return false;
        }
        env->GetByteArrayRegion(byteArray, 0, n, buf);
        sgf.append(reinterpret_cast<const char*>(&buf[0]), n);
    }
    inputStream.callMethod<void>("close");
    return true;
}
#endif

void AndroidUtils::releasePersistableUriPermission(
        [[maybe_unused]]const QString& uri)
{
#ifdef Q_OS_ANDROID
    if (QUrl(uri).isRelative())
        return;
    auto uriObj = getUriObj(uri);
    if (! uriObj.isValid())
        return;
    auto contentResolver = getContentResolver();
    if (! contentResolver.isValid())
        return;
    auto flagRead = QJniObject::getStaticField<jint>(
                "android/content/Intent", "FLAG_GRANT_READ_URI_PERMISSION");
    auto flagWrite = QJniObject::getStaticField<jint>(
                "android/content/Intent", "FLAG_GRANT_WRITE_URI_PERMISSION");
    contentResolver.callMethod<void>(
                "releasePersistableUriPermission", "(Landroid/net/Uri;I)V",
                uriObj.object(), flagRead | flagWrite);
#endif
}

bool AndroidUtils::save([[maybe_unused]]const QString& uri,
                        [[maybe_unused]]const QByteArray& array)
{
#ifdef Q_OS_ANDROID
    m_error.clear();
    auto contentResolver = getContentResolver();
    if (! contentResolver.isValid())
        return false;
    auto uriObj = getUriObj(uri);
    if (! uriObj.isValid())
        return false;
    auto outputStream = contentResolver.callObjectMethod(
                "openOutputStream",
                "(Landroid/net/Uri;Ljava/lang/String;)Ljava/io/OutputStream;",
                uriObj.object(),
                QJniObject::fromString("wt").object<jstring>());
    QJniEnvironment env;
    if (checkException())
        return false;
    if (! outputStream.isValid())
        return false;
    auto byteArray = env->NewByteArray(array.size());
    env->SetByteArrayRegion(byteArray, 0, array.size(),
                            reinterpret_cast<const jbyte*>(array.constData()));
    outputStream.callMethod<void>("write", "([B)V", byteArray);
    if (checkException())
        return false;
    outputStream.callMethod<void>("close");
    if (env->ExceptionCheck())
        return false;
    return true;
#else
    return false;
#endif
}

//-----------------------------------------------------------------------------
