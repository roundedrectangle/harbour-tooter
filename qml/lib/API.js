.pragma library

function setActiveAccount(account) {
    api.setConfig("instance", account.instance)
    api.setConfig("api_user_token", account.api_user_token)

    clearModels()
}

var tootParser = function(data){
    console.log(data)
    var ret = {};
    ret.id = data.id
    ret.content = data.content
    ret.created_at = data.created_at
    ret.in_reply_to_account_id = data.in_reply_to_account_id
    ret.in_reply_to_id = data.in_reply_to_id

    ret.user_id = data.account.id
    ret.user_locked = data.account.locked
    ret.username = data.account.username
    ret.display_name = data.account.display_name
    ret.avatar_static = data.account.avatar_static

    ret.favourited = data.favourited ? true : false
    ret.status_favourites_count = data.favourites_count ? data.favourites_count : 0

    ret.reblog = data.reblog ? true : false
    ret.reblogged = data.reblogged ? true : false
    ret.status_reblogs_count = data.reblogs_count ? data.reblogs_count : false

    ret.bookmarked = data.bookmarked ? true : false

    ret.muted = data.muted ? true : false
    ret.sensitive = data.sensitive ? true : false
    ret.visibility = data.visibility ? data.visibility : false

    console.log(ret)
}

var test = 1;

Qt.include("Mastodon.js")

var modelTLhome = Qt.createQmlObject('import QtQuick 2.0; ListModel {   }', Qt.application, 'InternalQmlObject');
var modelTLpublic = Qt.createQmlObject('import QtQuick 2.0; ListModel {   }', Qt.application, 'InternalQmlObject');
var modelTLlocal = Qt.createQmlObject('import QtQuick 2.0; ListModel {   }', Qt.application, 'InternalQmlObject');
var modelTLnotifications = Qt.createQmlObject('import QtQuick 2.0; ListModel {   }', Qt.application, 'InternalQmlObject');
var modelTLsearch = Qt.createQmlObject('import QtQuick 2.0; ListModel {   }', Qt.application, 'InternalQmlObject');
var modelTLbookmarks = Qt.createQmlObject('import QtQuick 2.0; ListModel {   }', Qt.application, 'InternalQmlObject');

function clearModels() {
    [modelTLhome, modelTLpublic, modelTLlocal, modelTLnotifications, modelTLsearch, modelTLbookmarks]
        .forEach(function(m) { m.clear() })
}

var notificationsList = []

var notificationGenerator = function(item){
    var notification;
    switch (item.urgency){
    case "normal":
        notification = Qt.createQmlObject('import org.nemomobile.notifications 1.0; Notification { category: "x-harbour.tooterb.activity"; appName: "Tooter β"; itemCount: 1; remoteActions: [ { "name": "default", "displayName": "Do something", "icon": "icon-s-certificates", "service": "ba.dysko.harbour.tooterb", "path": "/", "iface": "ba.dysko.harbour.tooterb", "method": "openapp", "arguments": [ "'+item.service+'", "'+item.key+'" ] }]; urgency: Notification.Normal;  }', Qt.application, 'InternalQmlObject');
        break;
    case "critical":
        notification = Qt.createQmlObject('import org.nemomobile.notifications 1.0; Notification { appName: "Tooter β"; itemCount: 1; remoteActions: [ { "name": "default", "displayName": "Do something", "icon": "icon-s-certificates", "service": "ba.dysko.harbour.tooterb", "path": "/", "iface": "ba.dysko.harbour.tooterb", "method": "openapp", "arguments": [ "'+item.service+'", "'+item.key+'" ] }]; urgency: Notification.Critical;  }', Qt.application, 'InternalQmlObject');
        break;
    default:
        notification = Qt.createQmlObject('import org.nemomobile.notifications 1.0; Notification { category: "x-harbour.tooterb.activity"; appName: "Tooter β"; itemCount: 1; remoteActions: [ { "name": "default", "displayName": "Do something", "icon": "icon-s-certificates", "service": "ba.dysko.harbour.tooterb", "path": "/", "iface": "ba.dysko.harbour.tooterb", "method": "openapp", "arguments": [ "'+item.service+'", "'+item.key+'" ] }]; urgency: Notification.Low;  }', Qt.application, 'InternalQmlObject');
    }

    console.log(JSON.stringify(notification.remoteActions[0].arguments))
    //Notifications.notify("Tooter β", "serverinfo.serverTitle", " new activity", false, "2015-10-15 00:00:00", "aaa")

    notification.timestamp = item.timestamp
    notification.summary = item.summary
    notification.body = item.body
    if(item.previewBody)
        notification.previewBody = item.previewBody;
    else
        notification.previewBody = item.body;
    if(item.previewSummary)
        notification.previewSummary = item.previewSummary;
    else
        notification.previewSummary = item.summary
    if(notification.replacesId){ notification.replacesId = 0 }
    notification.publish()
}

var notifier = function(item){

    item.content = item.content.replace(/(<([^>]+)>)/ig,"").replaceAll("&quot;", "\"")

    var msg;
    switch (item.type){
    case "favourite":
        msg = {
            urgency: "normal",
            timestamp: item.created_at,
            summary: (item.reblog_account_display_name !== "" ? item.reblog_account_display_name : '@'+item.reblog_account_username) + ' ' + qsTr("favourited"),
            body: item.content,
            service: 'toot',
            key: item.id
        }
        break;

    case "follow":
        msg = {
            urgency: "critical",
            timestamp: item.created_at,
            summary: (item.account_display_name !== "" ? item.account_display_name : '@'+item.account_username),
            body: qsTr("followed you"),
            service: 'profile',
            key: item.account_username
        }
        break;

    case "reblog":
        msg = {
            urgency: "low",
            timestamp: item.created_at,
            summary: (item.reblog_account_display_name !== "" ? item.reblog_account_display_name : '@'+item.reblog_account_username) + ' ' + qsTr("boosted"),
            body: item.content,
            service: 'toot',
            key: item.id
        }
        break;

    case "mention":
        msg = {
            urgency: "critical",
            timestamp: item.created_at,
            summary: (item.account_display_name !== "" ? item.account_display_name : '@'+item.account_username) + ' ' + qsTr("said"),
            body: item.content,
            previewBody: (item.account_display_name !== "" ? item.account_display_name : '@'+item.account_username) + ' ' + qsTr("said") + ': ' + item.content,
            service: 'toot',
            key: item.id
        }
        break;

    default:
        //console.log(JSON.stringify(messageObject.data))
        return;
    }
    notificationGenerator(msg)
}


var api;

function func() {
    console.log(api)
}
