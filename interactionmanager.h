#ifndef INTERACTIONMANAGER_H
#define INTERACTIONMANAGER_H

#include <QDebug>
#include <QObject>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWidget>
#include <QUrl>

// header files

// class declarations
class SettingsDBMS;
class UsersModel;
class PermissionsModel;

class InteractionManager : public QObject
{    
    // Macros
    Q_OBJECT

    Q_PROPERTY(QString userState READ getUserState NOTIFY userStateChanged)
    Q_PROPERTY(QString userName READ userName NOTIFY userChanged)
    Q_PROPERTY(QString userGroup READ userGroup NOTIFY userChanged)
    Q_PROPERTY(QString userPassword READ userPassword NOTIFY userChanged)
    Q_PROPERTY(int userPermissions READ userPermissions NOTIFY userChanged)

public:

    /////////////////////////

    // REVISIT 'How to bind a QML Property to a C++ Function' from Qt Wiki
    // WHEN IMPLEMENTING FEEDBACK FOR ADDING, EDITING, AND REMOVING A USER.
    // IMPROVE THE FEEDBACK

    /////////////////////////

    // Constructor.
    explicit InteractionManager();

    // The Q_INVOKABLE methods allow them to be invoked via the meta-object system.
    Q_INVOKABLE void userStateNotification();

    Q_INVOKABLE QString getUserName();
    Q_INVOKABLE QString getName(const QString& id);

    // The 'get' method are used to store data for retrieval.
    QString getUserState();
    QString userName() {return m_Name;}
    QString userGroup() {return m_Group;}
    QString userPassword() {return m_Password;}
    int userPermissions() {return m_Permissions;}

    void setContext(QQmlContext *root);

    bool userExists(const QString &user);
    bool checkValidPassword(const QString &user, const QString &password);

    Q_INVOKABLE bool checkUserExistance(const QString &id);

public slots:
    int init();
    void close();

    void userSelected(int index);
    void updateRootUser();
    void updateModel();

    void loginUser(const QString &user, const QString &password);

    // updating user data slots
    void editUsername(int user_index, const QString &newName);
    int getGroupIndex(int user_index);
    void selectGroup(int user_index, int group_index);
    void editPassword(int user_index, const QString &newPassword);
    void setPermission(int user_index, const QString name, bool state);

    void insertUser(const QString &id,const QString &name, const QString &password);

    void deleteUser(int user_index);


    // table view implementation methods
    void saveModelToDatabase();

signals:

    void userStateChanged();
    void userChanging();
    void userChanged();
    void selectUser(int select_index);
    void loginComplete(const QString& user, const QString &);

private:

    enum user_states_t {
        INVALID_CREDENTIALS,
        USER_EXISTS,
        USER_NONEXISTENT,
        USER_INSERTED,
        USER_UPDATED,
        USER_DELETED,
        USERID_EMPTY,
        USERNAME_EMPTY,
        USER_RESET
    };

    user_states_t m_userState;

    enum password_states_t {
        PASSWORD_EMPTY,
        PASSWORD_CONFIRMED,
        PASSWORD_RESET
    };

    password_states_t m_passwordState;

    // instance of MemberModel
    UsersModel *m_usersModel;

    PermissionsModel *m_permissionsModel;

    // default stored values
    const QString m_DEFAULT_GROUP = "Guest";
    const QString m_DEFAULT_PASSWORD = "password";
    const QString m_DEFAULT_PERMISSIONS = "24";

    QStringList m_GroupList;
    QString m_Name;
    QString m_Group;
    QString m_Password;
    int m_Permissions;
};

#endif // INTERACTIONMANAGER_H
