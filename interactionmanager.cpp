
#include "../controller/modconfig.h"
#include "../controller/settings.h"
#include "../controller/settingsio.h"
#include "../controller/websocketclient.hpp"
#include "xmui.h"
#include "usersmodel.h"
#include "interactionmanager.h"

InteractionManager::InteractionManager()
{
    // initialize user notifications
    m_userState = USER_RESET;
    userStateNotification();

    m_usersModel = new UsersModel(this);
    m_permissionsModel = new PermissionsModel(this);
    m_GroupList << "Administrator"
                << "Engineering"
                << "Maintenance"
                << "Operator";
}

void InteractionManager::setContext(QQmlContext *root)
{
    root->setContextProperty("usersui", this);
    m_usersModel->setContext(root);
    m_permissionsModel->setContext(root);
    root->setContextProperty("usergroups", QVariant::fromValue(m_GroupList));

}

int InteractionManager::init()
{
    updateModel();
    int index = m_usersModel->findUserIndex(ModuleConfiguration->currentUser());
    return index;
}

void InteractionManager::close()
{
    // to do check permissions
    saveModelToDatabase();
    QString current_user = ModuleConfiguration->currentUser();
    ModuleConfig_Class::Permissions permissions;
    m_usersModel->getUserPermissions(current_user, permissions);
    if(permissions != ModuleConfiguration->currentUserPermissions())
        ModuleConfiguration->setUser(current_user, permissions);
}

QString InteractionManager::getName(const QString& id)
{
    return m_usersModel->getUserName(id);
}

bool InteractionManager::userExists(const QString &user)
{
    return m_usersModel->hasUser(user);
}

bool InteractionManager::checkUserExistance(const QString &id)
{
    if (userExists(id) ){
        m_userState = USER_EXISTS;
        userStateNotification();
        m_userState = USER_RESET;
        return true;
    }else{
        m_userState = USER_NONEXISTENT;
        userStateNotification();
        m_userState = USER_RESET;
        return false;
    }
}

void InteractionManager::userStateNotification()
{
    emit userStateChanged();
}

QString InteractionManager::getUserState()
{
    switch(m_userState) {
    case INVALID_CREDENTIALS:
        return "Invalid credentials, please try again.";
    case USER_EXISTS:
        return "User exists.";
    case USER_NONEXISTENT:
        return "User doesn't exist.";
    case USERID_EMPTY:
        return "User ID field is empty.";
    case USER_RESET:
        return "";
    default:
        return "";
    }
}

bool InteractionManager::checkValidPassword(const QString &id, const QString& password)
{
    QString user_password = m_usersModel->getUserPassword(id);
    return(user_password == password);
}

void InteractionManager::loginUser(const QString &user, const QString &password)
{
    if ((user == NULL)) {
        m_userState = USERID_EMPTY;
        userStateNotification();
        m_userState = USER_RESET;
    }else if (userExists(user)) {
        if (checkValidPassword(user, password)) {
            //QStringList permissions;
            //m_usersModel->getUserPermissions(user, permissions);
            // WebSocketClient::set_user(user, permissions); //un comment after seq recompiled
            ModuleConfig_Class::Permissions permissions;
            m_usersModel->getUserPermissions(user, permissions);
            ModuleConfiguration->setUser(user,permissions);
            emit loginComplete(user, getName(user));
        } else {
            m_userState =  INVALID_CREDENTIALS;
            userStateNotification();
            m_userState = USER_RESET;
        }
    }
}

QString InteractionManager::getUserName()
{
    return ModuleConfiguration->currentUser();
}

void InteractionManager::updateRootUser()
{
    SettingsIONode usersNode, idNode, node;

    usersNode = Settings->settings.getNewKey("users");
    if(!usersNode.hasKey("ametinc")){
        idNode = usersNode.getNewKey("ametinc");
        node = idNode.getNewKey("name");
        node.setString("amet_support");
        node = idNode.getNewKey("group");
        node.setString("Administrator");
        node = idNode.getNewKey("permissions");
        node.setString("231");
        node = idNode.getNewKey("password");
        node.setString("ametxmxm");

        WebSocketClient::settings_merge(usersNode);
        WebSocketClient::settings_file_save();
    }
}

void InteractionManager::updateModel()
{
    m_usersModel->reset();

    SettingsIONode usersNode;
    usersNode = Settings->settings.getKey("users");
    for (SettingsIONode::iterator i = usersNode.begin(); i != usersNode.end(); ++i) {
        if (usersNode.hasKey(i.key())) {
           m_usersModel->add(i.key());
        }
    }
}

void InteractionManager::userSelected(int index)
{
    emit userChanging();
    m_Name = m_usersModel->getName(index);
    m_Group = m_usersModel->getGroup(index);
    m_Password = m_usersModel->getPassword(index);
    m_Permissions = m_usersModel->getPermissions(index).toInt();
    //    m_permissionsModel->((ModuleConfig_Class::Permissions)user_permission);
    emit userChanged();
}

void InteractionManager::saveModelToDatabase()
{
    SettingsIONode usersNode = Settings->settings.getKey("users");
    WebSocketClient::settings_replace(usersNode);
    WebSocketClient::settings_file_save();
}

void InteractionManager::editUsername(int user_index, const QString &newName)
{
    m_usersModel->setName(user_index, newName);
    m_Name = m_usersModel->getName(user_index);
    emit userChanged();
}

int InteractionManager::getGroupIndex(int user_index)
{
    QString group = m_usersModel->getGroup(user_index);
    return m_GroupList.indexOf(group);
}

void InteractionManager::selectGroup(int user_index, int group_index)
{
    QString group = m_GroupList.at(group_index);

    m_usersModel->setGroup(user_index, group);
    m_Group = m_usersModel->getGroup(user_index);
    emit userChanged();
}

void InteractionManager::editPassword(int user_index, const QString &newPassword)
{
    m_usersModel->setPassword(user_index, newPassword);
    m_Password = m_usersModel->getPassword(user_index);
    emit userChanged();
}

void InteractionManager::setPermission(int user_index, const QString name, bool state)
{
    int permissionsInt = m_usersModel->getPermissions(user_index).toInt();
    QStringList permissions = permissions_to_qstringlist((ModuleConfig_Class::Permissions) permissionsInt);
    if(state){
        permissions.append(name);
    }else{
        permissions.removeAll(name);
    }
    m_Permissions = qstringlist_to_permissions(permissions);
    m_usersModel->setPermissions(user_index, QString::number(m_Permissions));
    emit userChanged();

}

void InteractionManager::insertUser(const QString &id,const QString &name,const QString &password)
{
    int index = m_usersModel->add(id);
    m_usersModel->setName(index, name);
    m_usersModel->setPassword(index, password);
    m_usersModel->setGroup(index, "None");
    m_usersModel->setPermissions(index, "2");
    emit selectUser(index);
}

void InteractionManager::deleteUser(int user_index)
{
    m_usersModel->removeUser(user_index);
    if(m_usersModel->rowCount() > user_index)
        userSelected(user_index);
    else
        userSelected(user_index - 1);

}



