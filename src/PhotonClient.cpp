#include "PhotonClient.h"
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/color.hpp>
#include "PhotonView.h"
#include "PhotonTransformView.h"
#include "PhotonRigidbodyView.h"

using namespace godot;

static PhotonClient* plugin_singleton = nullptr;

PhotonClient* PhotonClient::get_singleton() {
    return plugin_singleton;
}

void PhotonClient::_bind_methods() {
    ClassDB::bind_method(D_METHOD("connect_to_server", "app_id", "app_version", "region"), &PhotonClient::connect_to_server, DEFVAL(""), DEFVAL(""), DEFVAL("eu"));
    ClassDB::bind_method(D_METHOD("join_lobby"), &PhotonClient::join_lobby);
    ClassDB::bind_method(D_METHOD("get_state"), &PhotonClient::get_state);
    ClassDB::bind_method(D_METHOD("get_ping"), &PhotonClient::get_ping);
    ClassDB::bind_method(D_METHOD("get_room_list"), &PhotonClient::get_room_list);

    ClassDB::bind_method(D_METHOD("create_room", "room_name", "max_players", "is_open", "is_visible", "custom_properties"), &PhotonClient::create_room, DEFVAL(4), DEFVAL(true), DEFVAL(true), DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("join_room", "room_name"), &PhotonClient::join_room);
    ClassDB::bind_method(D_METHOD("join_random_room"), &PhotonClient::join_random_room);
    ClassDB::bind_method(D_METHOD("join_or_create_room", "room_name"), &PhotonClient::join_or_create_room);
    ClassDB::bind_method(D_METHOD("leave_room"), &PhotonClient::leave_room);

    ClassDB::bind_method(D_METHOD("get_local_player_id"), &PhotonClient::get_local_player_id);
    ClassDB::bind_method(D_METHOD("is_master_client"), &PhotonClient::is_master_client);
    ClassDB::bind_method(D_METHOD("set_master_client", "player_id"), &PhotonClient::set_master_client);
    ClassDB::bind_method(D_METHOD("get_master_client_id"), &PhotonClient::get_master_client_id);
    ClassDB::bind_method(D_METHOD("get_player_list"), &PhotonClient::get_player_list);

    ClassDB::bind_method(D_METHOD("instantiate", "prefab_path", "position", "rotation"), &PhotonClient::instantiate, DEFVAL(Vector3()), DEFVAL(Vector3()));
    ClassDB::bind_method(D_METHOD("destroy", "target_node"), &PhotonClient::destroy);
    ClassDB::bind_method(D_METHOD("load_network_scene", "scene_path"), &PhotonClient::load_network_scene);
    ClassDB::bind_method(D_METHOD("raise_custom_event", "event_code", "data"), &PhotonClient::raise_custom_event);

    ClassDB::bind_method(D_METHOD("set_player_property", "key", "value"), &PhotonClient::set_player_property);
    ClassDB::bind_method(D_METHOD("get_player_property", "player_id", "key"), &PhotonClient::get_player_property);
    ClassDB::bind_method(D_METHOD("set_room_property", "key", "value"), &PhotonClient::set_room_property);
    ClassDB::bind_method(D_METHOD("get_room_property", "key"), &PhotonClient::get_room_property);

    ClassDB::bind_method(D_METHOD("set_offline_mode", "offline"), &PhotonClient::set_offline_mode);
    ClassDB::bind_method(D_METHOD("get_offline_mode"), &PhotonClient::get_offline_mode);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "offline_mode"), "set_offline_mode", "get_offline_mode");

    ADD_SIGNAL(MethodInfo("connected_to_master"));
    ADD_SIGNAL(MethodInfo("disconnected"));
    ADD_SIGNAL(MethodInfo("connection_error", PropertyInfo(Variant::INT, "error_code")));
    ADD_SIGNAL(MethodInfo("lobby_joined"));
    ADD_SIGNAL(MethodInfo("room_list_updated", PropertyInfo(Variant::DICTIONARY, "rooms")));
    ADD_SIGNAL(MethodInfo("room_created", PropertyInfo(Variant::STRING, "room_name")));
    ADD_SIGNAL(MethodInfo("room_joined", PropertyInfo(Variant::STRING, "room_name")));
    ADD_SIGNAL(MethodInfo("room_left"));
    ADD_SIGNAL(MethodInfo("room_failed", PropertyInfo(Variant::INT, "error_code"), PropertyInfo(Variant::STRING, "error_message")));
    ADD_SIGNAL(MethodInfo("player_joined", PropertyInfo(Variant::INT, "player_id")));
    ADD_SIGNAL(MethodInfo("player_left", PropertyInfo(Variant::INT, "player_id")));
    ADD_SIGNAL(MethodInfo("master_client_switched", PropertyInfo(Variant::INT, "new_master_id"), PropertyInfo(Variant::INT, "old_master_id")));
    ADD_SIGNAL(MethodInfo("network_scene_loaded", PropertyInfo(Variant::STRING, "scene_path")));
    ADD_SIGNAL(MethodInfo("custom_event_received", PropertyInfo(Variant::INT, "player_id"), PropertyInfo(Variant::INT, "event_code"), PropertyInfo(Variant::DICTIONARY, "data")));
    ADD_SIGNAL(MethodInfo("player_property_changed", PropertyInfo(Variant::INT, "player_id"), PropertyInfo(Variant::STRING, "key"), PropertyInfo(Variant::NIL, "value")));
    ADD_SIGNAL(MethodInfo("room_property_changed", PropertyInfo(Variant::STRING, "key"), PropertyInfo(Variant::NIL, "value")));

    BIND_ENUM_CONSTANT(STATE_CONNECTING_TO_NAMESERVER);
    BIND_ENUM_CONSTANT(STATE_CONNECTED_TO_NAMESERVER);
    BIND_ENUM_CONSTANT(STATE_DISCONNECTING_FROM_NAMESERVER);
    BIND_ENUM_CONSTANT(STATE_CONNECTING_TO_MASTERSERVER);
    BIND_ENUM_CONSTANT(STATE_CONNECTED_TO_MASTERSERVER);
    BIND_ENUM_CONSTANT(STATE_JOINED_LOBBY);

    BIND_ENUM_CONSTANT(EVENT_TRANSFORM_SYNC);
    BIND_ENUM_CONSTANT(EVENT_INSTANTIATE);
    BIND_ENUM_CONSTANT(EVENT_DESTROY);
    BIND_ENUM_CONSTANT(EVENT_SET_PROPERTIES);
    BIND_ENUM_CONSTANT(EVENT_TRANSFER_OWNERSHIP);
    BIND_ENUM_CONSTANT(EVENT_RPC);
    BIND_ENUM_CONSTANT(EVENT_LOAD_SCENE);
    BIND_ENUM_CONSTANT(EVENT_RIGIDBODY_SYNC);
    BIND_ENUM_CONSTANT(EVENT_ANIMATOR_SYNC);
}

PhotonClient::PhotonClient() {
    m_client = nullptr;
    plugin_singleton = this;
    offline_mode = false;
    instantiated_count = 0;
    set_process(true);
}

PhotonClient::~PhotonClient() {
    if (plugin_singleton == this) plugin_singleton = nullptr;
    if (m_client) {
        m_client->disconnect();
        delete m_client;
    }
}

void PhotonClient::_process(double delta) {
    if (m_client) m_client->service();
}

void PhotonClient::set_offline_mode(bool offline) { offline_mode = offline; }
bool PhotonClient::get_offline_mode() const { return offline_mode; }

void PhotonClient::connect_to_server(const String& p_app_id, const String& p_app_version, const String& p_region) {
    if (offline_mode) {
        UtilityFunctions::print("PhotonClient: Offline mode active. Bypassing connection...");
        emit_signal("connected_to_master");
        emit_signal("lobby_joined");
        return;
    }

    ProjectSettings *settings = ProjectSettings::get_singleton();
    if (!p_app_id.is_empty()) current_app_id = p_app_id;
    else current_app_id = settings->get_setting("addons/photon/connection/app_id");

    if (!p_app_version.is_empty()) current_app_version = p_app_version;
    else current_app_version = settings->get_setting("addons/photon/connection/app_version");

    if (current_app_id.is_empty()) {
        UtilityFunctions::printerr("PhotonClient Error: AppID is empty!");
        return;
    }

    if (m_client) delete m_client;

    ExitGames::Common::JString appID(current_app_id.utf8().get_data());
    ExitGames::Common::JString appVer(current_app_version.utf8().get_data());
    ExitGames::Common::JString regionStr(p_region.utf8().get_data());

    m_client = new ExitGames::LoadBalancing::Client(*this, appID, appVer, ExitGames::Photon::ConnectionProtocol::UDP);
    
    UtilityFunctions::print("PhotonClient: Initiating connection to region: ", p_region);
    
    m_client->connect();
    
    set_process(true);
}

bool PhotonClient::join_lobby() {
    if (!m_client) return false;
    bool success = m_client->opJoinLobby();
    if (success) UtilityFunctions::print("PhotonClient: Request to join lobby accepted!");
    return success;
}

int PhotonClient::get_state() {
    if (offline_mode) return STATE_JOINED_LOBBY;
    if (!m_client) return 0;
    return m_client->getState();
}

int PhotonClient::get_ping() {
    if (!m_client) return 0;
    return m_client->getRoundTripTime();
}

void PhotonClient::create_room(const String& room_name, int max_players, bool is_open, bool is_visible, const Dictionary& custom_properties) {
    if (offline_mode) {
        UtilityFunctions::print("PhotonClient: Offline mode. Simulating room join...");
        emit_signal("room_created", room_name);
        return;
    }
    if (!m_client) return;
    
    UtilityFunctions::print("PhotonClient: Creating room '", room_name, "' with advanced options...");

    ExitGames::Common::JString j_room_name(room_name.utf8().get_data());
    ExitGames::LoadBalancing::RoomOptions options;
    options.setMaxPlayers((nByte)max_players);
    options.setIsOpen(is_open);
    options.setIsVisible(is_visible);
    
    if (!custom_properties.is_empty()) {
        ExitGames::Common::Hashtable roomProps = _dict_to_hashtable(custom_properties);
        options.setCustomRoomProperties(roomProps);
        
        ExitGames::Common::JVector<ExitGames::Common::JString> propsListedInLobby;
        ExitGames::Common::JVector<ExitGames::Common::Object> keys = roomProps.getKeys();
        
        for (unsigned int i = 0; i < keys.getSize(); i++) {
            if (keys[i].getType() == ExitGames::Common::TypeCode::STRING) {
                propsListedInLobby.addElement(ExitGames::Common::ValueObject<ExitGames::Common::JString>(keys[i]).getDataCopy());
            }
        }
        options.setPropsListedInLobby(propsListedInLobby);
    }
    m_client->opCreateRoom(j_room_name, options);
}

void PhotonClient::join_room(const String& room_name) {
    if (offline_mode) {
        UtilityFunctions::print("PhotonClient: Offline mode. Simulating room join...");
        emit_signal("room_joined", room_name);
        return;
    }
    if (!m_client) return;
    UtilityFunctions::print("PhotonClient: Joining room -> ", room_name);
    
    ExitGames::Common::JString j_room_name(room_name.utf8().get_data());
    m_client->opJoinRoom(j_room_name);
}

void PhotonClient::join_random_room() {
    if (!m_client) return;
    UtilityFunctions::print("PhotonClient: Searching for a random room...");
    m_client->opJoinRandomRoom();
}

void PhotonClient::join_or_create_room(const String& room_name) {
    if (!m_client) return;
    UtilityFunctions::print("PhotonClient: Join or Create room -> ", room_name);
    ExitGames::Common::JString j_room_name(room_name.utf8().get_data());
    ExitGames::LoadBalancing::RoomOptions options;
    options.setMaxPlayers(4);
    m_client->opJoinOrCreateRoom(j_room_name, options);
}

void PhotonClient::leave_room() {
    if (!m_client) return;
    UtilityFunctions::print("PhotonClient: Leaving room...");
    m_client->opLeaveRoom();
}

Dictionary PhotonClient::get_room_list() {
    return cached_room_list;
}

int PhotonClient::get_local_player_id() {
    if (offline_mode) return 1;
    if (!m_client) return 0;
    return m_client->getLocalPlayer().getNumber();
}

bool PhotonClient::is_master_client() {
    if (offline_mode) return true;
    if (!m_client) return false;
    return m_client->getLocalPlayer().getIsMasterClient();
}

bool PhotonClient::set_master_client(int player_id) {
    if (!m_client) return false;
    const ExitGames::LoadBalancing::Player* target_player = m_client->getCurrentlyJoinedRoom().getPlayerForNumber(player_id);
    
    if (!target_player) {
        UtilityFunctions::printerr("Photon Error: Cannot set Master Client. Player ", player_id, " not found.");
        return false;
    }
    return m_client->getCurrentlyJoinedRoom().setMasterClient(*target_player);
}

int PhotonClient::get_master_client_id() {
    if (!m_client) return 0;
    return m_client->getCurrentlyJoinedRoom().getMasterClientID();
}

Dictionary PhotonClient::get_player_list() {
    Dictionary result;
    if (!m_client) return result;

    const ExitGames::Common::JVector<ExitGames::LoadBalancing::Player*>& players = m_client->getCurrentlyJoinedRoom().getPlayers();
    for (unsigned int i = 0; i < players.getSize(); i++) {
        const ExitGames::LoadBalancing::Player* p = players[i];
        if (!p) continue;

        Dictionary p_info;
        p_info["name"] = String(p->getName().UTF8Representation().cstr());
        p_info["is_master"] = p->getIsMasterClient();
        p_info["is_local"] = (p->getNumber() == m_client->getLocalPlayer().getNumber());
        p_info["custom_properties"] = _hashtable_to_dict(p->getCustomProperties());

        result[p->getNumber()] = p_info;
    }
    return result;
}

Node* PhotonClient::instantiate(const String& prefab_path, const Vector3& position, const Vector3& rotation) {
    if (!m_client && !offline_mode) {
        UtilityFunctions::printerr("Photon Error: Cannot instantiate, client is null.");
        return nullptr;
    }

    int player_id = get_local_player_id();
    if (player_id <= 0) {
        UtilityFunctions::printerr("Photon Error: Cannot instantiate, you are not in a room!");
        return nullptr;
    }

    Ref<PackedScene> scene = ResourceLoader::get_singleton()->load(prefab_path);
    if (scene.is_null()) {
        UtilityFunctions::printerr("Photon Error: Failed to load scene at ", prefab_path);
        return nullptr;
    }

    Node* instance = scene->instantiate();
    PhotonView* view = _find_photon_view(instance);
    
    if (!view) {
        UtilityFunctions::printerr("Photon Error: The prefab ", prefab_path, " DOES NOT have a PhotonView component! Destruction initiated.");
        instance->queue_free();
        return nullptr;
    }

    instantiated_count++;
    int new_view_id = (player_id * 1000) + instantiated_count;

    view->set_view_id(new_view_id);
    view->set_owner_id(player_id);

    if (Node3D* spatial = Object::cast_to<Node3D>(instance)) {
        spatial->set_position(position);
        spatial->set_rotation(rotation);
    }

    this->add_child(instance);

    Dictionary spawn_data;
    spawn_data["path"] = prefab_path;
    spawn_data["vid"] = new_view_id;
    spawn_data["oid"] = player_id;
    spawn_data["pos"] = position;
    spawn_data["rot"] = rotation;

    raise_custom_event(EVENT_INSTANTIATE, spawn_data, PhotonView::RPC_OTHERS_BUFFERED);
    UtilityFunctions::print("Photon: Instantiated local object ", prefab_path, " with ViewID ", new_view_id);
    return instance;
}

void PhotonClient::destroy(Node* target_node) {
    if (!m_client && !offline_mode) return;
    if (!target_node) return;

    PhotonView* view = _find_photon_view(target_node);
    if (!view) {
        UtilityFunctions::printerr("Photon Error: Cannot destroy node without PhotonView.");
        return;
    }

    if (!view->is_mine()) {
        UtilityFunctions::printerr("Photon Error: Cannot destroy an object you don't own! ViewID: ", view->get_view_id());
        return;
    }

    int vid = view->get_view_id();
    Dictionary destroy_data;
    destroy_data["vid"] = vid;

    raise_custom_event(EVENT_DESTROY, destroy_data, PhotonView::RPC_OTHERS_BUFFERED);
    UtilityFunctions::print("Photon: Destroyed local object with ViewID ", vid);
    target_node->queue_free();
}

void PhotonClient::load_network_scene(const String& scene_path) {
    if (!m_client && !offline_mode) return;
    if (!is_master_client()) return;

    if (is_inside_tree()) {
        TypedArray<Node> nodes = get_tree()->get_nodes_in_group(StringName("PhotonNetworked"));
        for (int i = 0; i < nodes.size(); i++) {
            Node* pv = Object::cast_to<Node>(nodes[i]);
            if (pv && pv->get_parent()) pv->get_parent()->queue_free();
        }
    }

    Dictionary data;
    data["path"] = scene_path;
    raise_custom_event(EVENT_LOAD_SCENE, data, PhotonView::RPC_ALL_BUFFERED);
}

void PhotonClient::raise_custom_event(int event_code, const Dictionary& data, int rpc_target, bool reliable) {
    if (offline_mode) {
        if (rpc_target == PhotonView::RPC_ALL || 
            rpc_target == PhotonView::RPC_ALL_BUFFERED || 
            rpc_target == PhotonView::RPC_MASTER_CLIENT) {
            _process_custom_event(1, event_code, data);
        }
        return;
    }

    if (!m_client) return;

    ExitGames::Common::Hashtable evData = _dict_to_hashtable(data);
    ExitGames::LoadBalancing::RaiseEventOptions options;

    switch (rpc_target) {
        case PhotonView::RPC_ALL:
            options.setReceiverGroup(ExitGames::Lite::ReceiverGroup::ALL);
            break;
        case PhotonView::RPC_OTHERS:
            options.setReceiverGroup(ExitGames::Lite::ReceiverGroup::OTHERS);
            break;
        case PhotonView::RPC_MASTER_CLIENT:
            options.setReceiverGroup(ExitGames::Lite::ReceiverGroup::MASTER_CLIENT);
            break;
        case PhotonView::RPC_ALL_BUFFERED:
            options.setReceiverGroup(ExitGames::Lite::ReceiverGroup::ALL);
            options.setEventCaching(ExitGames::Lite::EventCache::ADD_TO_ROOM_CACHE);
            break;
        case PhotonView::RPC_OTHERS_BUFFERED:
            options.setReceiverGroup(ExitGames::Lite::ReceiverGroup::OTHERS);
            options.setEventCaching(ExitGames::Lite::EventCache::ADD_TO_ROOM_CACHE);
            break;
    }
    m_client->opRaiseEvent(reliable, evData, (nByte)event_code, options);
}

void PhotonClient::set_player_property(const String& key, const Variant& value) {
    int pid = get_local_player_id();
    if (pid <= 0) return;

    if (!player_properties.has(pid)) {
        player_properties[pid] = Dictionary();
    }
    Dictionary props = player_properties[pid];
    props[key] = value;
    player_properties[pid] = props;
    
    emit_signal("player_property_changed", pid, key, value);

    Dictionary data;
    data["t"] = 0;
    data["p"] = pid;
    data["k"] = key;
    data["v"] = value;
    raise_custom_event(EVENT_SET_PROPERTIES, data, PhotonView::RPC_OTHERS_BUFFERED);
}

Variant PhotonClient::get_player_property(int player_id, const String& key) {
    if (player_properties.has(player_id)) {
        Dictionary props = player_properties[player_id];
        if (props.has(key)) return props[key];
    }
    return Variant();
}

void PhotonClient::set_room_property(const String& key, const Variant& value) {
    room_properties[key] = value;
    emit_signal("room_property_changed", key, value);

    Dictionary data;
    data["t"] = 1;
    data["k"] = key;
    data["v"] = value;
    raise_custom_event(EVENT_SET_PROPERTIES, data, PhotonView::RPC_OTHERS_BUFFERED);
}

Variant PhotonClient::get_room_property(const String& key) {
    if (room_properties.has(key)) return room_properties[key];
    return Variant();
}

ExitGames::Common::Hashtable PhotonClient::_dict_to_hashtable(const Dictionary& dict) {
    ExitGames::Common::Hashtable hash;
    Array keys = dict.keys();

    for (int i = 0; i < keys.size(); i++) {
        Variant key = keys[i];
        if (key.get_type() != Variant::STRING) continue;

        ExitGames::Common::JString jKey(String(key).utf8().get_data());
        Variant value = dict[key];

        switch (value.get_type()) {
            case Variant::INT: hash.put(jKey, (int)value); break;
            case Variant::FLOAT: hash.put(jKey, (float)value); break;
            case Variant::BOOL: hash.put(jKey, (bool)value); break;
            case Variant::STRING: hash.put(jKey, ExitGames::Common::JString(String(value).utf8().get_data())); break;
            case Variant::VECTOR2: {
                Vector2 v = value;
                ExitGames::Common::Hashtable v2_hash;
                v2_hash.put(ExitGames::Common::JString("v2x"), (float)v.x);
                v2_hash.put(ExitGames::Common::JString("v2y"), (float)v.y);
                hash.put(jKey, v2_hash);
                break;
            }
            case Variant::VECTOR3: {
                Vector3 v = value;
                ExitGames::Common::Hashtable v3_hash;
                v3_hash.put(ExitGames::Common::JString("v3x"), (float)v.x);
                v3_hash.put(ExitGames::Common::JString("v3y"), (float)v.y);
                v3_hash.put(ExitGames::Common::JString("v3z"), (float)v.z);
                hash.put(jKey, v3_hash);
                break;
            }
            case Variant::ARRAY: {
                Array arr = value;
                ExitGames::Common::Hashtable arr_hash;
                arr_hash.put(ExitGames::Common::JString("is_array"), true);
                for (int j = 0; j < arr.size(); j++) {
                    Variant item = arr[j];
                    ExitGames::Common::JString idx(String::num_int64(j).utf8().get_data());
                    
                    if (item.get_type() == Variant::INT) arr_hash.put(idx, (int)item);
                    else if (item.get_type() == Variant::FLOAT) arr_hash.put(idx, (float)item);
                    else if (item.get_type() == Variant::STRING) arr_hash.put(idx, ExitGames::Common::JString(String(item).utf8().get_data()));
                    else if (item.get_type() == Variant::BOOL) arr_hash.put(idx, (bool)item);
                    else if (item.get_type() == Variant::VECTOR3) {
                        Vector3 v = item;
                        ExitGames::Common::Hashtable v3_hash;
                        v3_hash.put(ExitGames::Common::JString("is_v3"), true);
                        v3_hash.put(ExitGames::Common::JString("x"), (float)v.x);
                        v3_hash.put(ExitGames::Common::JString("y"), (float)v.y);
                        v3_hash.put(ExitGames::Common::JString("z"), (float)v.z);
                        arr_hash.put(idx, v3_hash);
                    }
                    else if (item.get_type() == Variant::COLOR) {
                        Color c = item;
                        ExitGames::Common::Hashtable c_hash;
                        c_hash.put(ExitGames::Common::JString("is_color"), true);
                        c_hash.put(ExitGames::Common::JString("r"), (float)c.r);
                        c_hash.put(ExitGames::Common::JString("g"), (float)c.g);
                        c_hash.put(ExitGames::Common::JString("b"), (float)c.b);
                        c_hash.put(ExitGames::Common::JString("a"), (float)c.a);
                        arr_hash.put(idx, c_hash);
                    }
                }
                hash.put(jKey, arr_hash);
                break;
            }
            case Variant::DICTIONARY: {
                ExitGames::Common::Hashtable dict_hash = _dict_to_hashtable((Dictionary)value);
                hash.put(jKey, dict_hash);
                break;
            }
            default:
                UtilityFunctions::printerr("PhotonClient: Unsupported Variant type in Dictionary for key: ", key);
                break;
        }
    }
    return hash;
}

Dictionary PhotonClient::_hashtable_to_dict(const ExitGames::Common::Hashtable& table) {
    Dictionary dict;
    ExitGames::Common::JVector<ExitGames::Common::Object> keys = table.getKeys();

    for (unsigned int i = 0; i < keys.getSize(); i++) {
        ExitGames::Common::Object k = keys[i];
        const ExitGames::Common::Object* v = table.getValue(k);
        if (!v || k.getType() != ExitGames::Common::TypeCode::STRING) continue;

        String godot_key = String(((ExitGames::Common::JString)ExitGames::Common::ValueObject<ExitGames::Common::JString>(k).getDataCopy()).UTF8Representation().cstr());

        switch (v->getType()) {
            case ExitGames::Common::TypeCode::INTEGER: dict[godot_key] = ExitGames::Common::ValueObject<int>(*v).getDataCopy(); break;
            case ExitGames::Common::TypeCode::FLOAT: dict[godot_key] = ExitGames::Common::ValueObject<float>(*v).getDataCopy(); break;
            case ExitGames::Common::TypeCode::BOOLEAN: dict[godot_key] = ExitGames::Common::ValueObject<bool>(*v).getDataCopy(); break;
            case ExitGames::Common::TypeCode::STRING: dict[godot_key] = String(((ExitGames::Common::JString)ExitGames::Common::ValueObject<ExitGames::Common::JString>(*v).getDataCopy()).UTF8Representation().cstr()); break;
            case ExitGames::Common::TypeCode::HASHTABLE: {
                ExitGames::Common::Hashtable nested = ExitGames::Common::ValueObject<ExitGames::Common::Hashtable>(*v).getDataCopy();
                
                if (nested.contains(ExitGames::Common::JString("v2x"))) {
                    float x = 0, y = 0;
                    const ExitGames::Common::Object* ox = nested.getValue(ExitGames::Common::JString("v2x"));
                    const ExitGames::Common::Object* oy = nested.getValue(ExitGames::Common::JString("v2y"));
                    if (ox) x = ExitGames::Common::ValueObject<float>(*ox).getDataCopy();
                    if (oy) y = ExitGames::Common::ValueObject<float>(*oy).getDataCopy();
                    dict[godot_key] = Vector2(x, y);
                }
                else if (nested.contains(ExitGames::Common::JString("v3x"))) {
                    float x = 0, y = 0, z = 0;
                    const ExitGames::Common::Object* ox = nested.getValue(ExitGames::Common::JString("v3x"));
                    const ExitGames::Common::Object* oy = nested.getValue(ExitGames::Common::JString("v3y"));
                    const ExitGames::Common::Object* oz = nested.getValue(ExitGames::Common::JString("v3z"));
                    if (ox) x = ExitGames::Common::ValueObject<float>(*ox).getDataCopy();
                    if (oy) y = ExitGames::Common::ValueObject<float>(*oy).getDataCopy();
                    if (oz) z = ExitGames::Common::ValueObject<float>(*oz).getDataCopy();
                    dict[godot_key] = Vector3(x, y, z);
                }
                else if (nested.contains(ExitGames::Common::JString("is_v3"))) {
                    float x = 0, y = 0, z = 0;
                    const ExitGames::Common::Object* ox = nested.getValue(ExitGames::Common::JString("x"));
                    const ExitGames::Common::Object* oy = nested.getValue(ExitGames::Common::JString("y"));
                    const ExitGames::Common::Object* oz = nested.getValue(ExitGames::Common::JString("z"));
                    if (ox) x = ExitGames::Common::ValueObject<float>(*ox).getDataCopy();
                    if (oy) y = ExitGames::Common::ValueObject<float>(*oy).getDataCopy();
                    if (oz) z = ExitGames::Common::ValueObject<float>(*oz).getDataCopy();
                    dict[godot_key] = Vector3(x, y, z);
                }
                else if (nested.contains(ExitGames::Common::JString("is_color"))) {
                    float r = 0, g = 0, b = 0, a = 1;
                    const ExitGames::Common::Object* or_ = nested.getValue(ExitGames::Common::JString("r"));
                    const ExitGames::Common::Object* og_ = nested.getValue(ExitGames::Common::JString("g"));
                    const ExitGames::Common::Object* ob_ = nested.getValue(ExitGames::Common::JString("b"));
                    const ExitGames::Common::Object* oa_ = nested.getValue(ExitGames::Common::JString("a"));
                    if (or_) r = ExitGames::Common::ValueObject<float>(*or_).getDataCopy();
                    if (og_) g = ExitGames::Common::ValueObject<float>(*og_).getDataCopy();
                    if (ob_) b = ExitGames::Common::ValueObject<float>(*ob_).getDataCopy();
                    if (oa_) a = ExitGames::Common::ValueObject<float>(*oa_).getDataCopy();
                    dict[godot_key] = Color(r, g, b, a);
                }
                else if (nested.contains(ExitGames::Common::JString("is_array"))) {
                    Array gd_arr;
                    int count = nested.getSize() - 1;
                    for (int j = 0; j < count; j++) {
                        ExitGames::Common::JString idx(String::num_int64(j).utf8().get_data());
                        const ExitGames::Common::Object* item = nested.getValue(idx);
                        if (item) {
                            switch (item->getType()) {
                                case ExitGames::Common::TypeCode::INTEGER: gd_arr.push_back(ExitGames::Common::ValueObject<int>(*item).getDataCopy()); break;
                                case ExitGames::Common::TypeCode::FLOAT: gd_arr.push_back(ExitGames::Common::ValueObject<float>(*item).getDataCopy()); break;
                                case ExitGames::Common::TypeCode::BOOLEAN: gd_arr.push_back(ExitGames::Common::ValueObject<bool>(*item).getDataCopy()); break;
                                case ExitGames::Common::TypeCode::STRING: gd_arr.push_back(String(((ExitGames::Common::JString)ExitGames::Common::ValueObject<ExitGames::Common::JString>(*item).getDataCopy()).UTF8Representation().cstr())); break;
                                case ExitGames::Common::TypeCode::HASHTABLE: {
                                    ExitGames::Common::Hashtable item_hash = ExitGames::Common::ValueObject<ExitGames::Common::Hashtable>(*item).getDataCopy();
                                    if (item_hash.contains(ExitGames::Common::JString("is_v3"))) {
                                        float x = 0, y = 0, z = 0;
                                        const ExitGames::Common::Object* ox = item_hash.getValue(ExitGames::Common::JString("x"));
                                        const ExitGames::Common::Object* oy = item_hash.getValue(ExitGames::Common::JString("y"));
                                        const ExitGames::Common::Object* oz = item_hash.getValue(ExitGames::Common::JString("z"));
                                        if (ox) x = ExitGames::Common::ValueObject<float>(*ox).getDataCopy();
                                        if (oy) y = ExitGames::Common::ValueObject<float>(*oy).getDataCopy();
                                        if (oz) z = ExitGames::Common::ValueObject<float>(*oz).getDataCopy();
                                        gd_arr.push_back(Vector3(x, y, z));
                                    } else if (item_hash.contains(ExitGames::Common::JString("is_color"))) {
                                        float r = 0, g = 0, b = 0, a = 1;
                                        const ExitGames::Common::Object* or_ = item_hash.getValue(ExitGames::Common::JString("r"));
                                        const ExitGames::Common::Object* og_ = item_hash.getValue(ExitGames::Common::JString("g"));
                                        const ExitGames::Common::Object* ob_ = item_hash.getValue(ExitGames::Common::JString("b"));
                                        const ExitGames::Common::Object* oa_ = item_hash.getValue(ExitGames::Common::JString("a"));
                                        if (or_) r = ExitGames::Common::ValueObject<float>(*or_).getDataCopy();
                                        if (og_) g = ExitGames::Common::ValueObject<float>(*og_).getDataCopy();
                                        if (ob_) b = ExitGames::Common::ValueObject<float>(*ob_).getDataCopy();
                                        if (oa_) a = ExitGames::Common::ValueObject<float>(*oa_).getDataCopy();
                                        gd_arr.push_back(Color(r, g, b, a));
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    dict[godot_key] = gd_arr;
                }
                else {
                    dict[godot_key] = _hashtable_to_dict(nested);
                }
                break;
            }
        }
    }
    return dict;
}

PhotonView* PhotonClient::_find_photon_view(Node* parent) {
    if (!parent) return nullptr;
    if (PhotonView* pv = Object::cast_to<PhotonView>(parent)) return pv;
    for (int i = 0; i < parent->get_child_count(); i++) {
        if (PhotonView* pv = _find_photon_view(parent->get_child(i))) return pv;
    }
    return nullptr;
}

void PhotonClient::_cleanup_room_objects() {
    if (!is_inside_tree()) return;
    TypedArray<Node> nodes = get_tree()->get_nodes_in_group(StringName("PhotonNetworked"));
    for (int i = 0; i < nodes.size(); i++) {
        Node* pv = Object::cast_to<Node>(nodes[i]);
        if (pv && pv->get_parent()) pv->get_parent()->queue_free();
    }
    player_properties.clear();
    room_properties.clear();
    UtilityFunctions::print("Photon: Local room objects cleaned up.");
}

void PhotonClient::_process_custom_event(int playerNr, int eventCode, const Dictionary& godot_dict) {
    if (eventCode == EVENT_TRANSFORM_SYNC) {
        int vid = godot_dict["vid"];
        Variant pos = godot_dict.has("p") ? godot_dict["p"] : Variant();
        Variant rot = godot_dict.has("r") ? godot_dict["r"] : Variant();
        Variant scale = godot_dict.has("s") ? godot_dict["s"] : Variant();

        if (PhotonView* view = PhotonView::get_view(vid)) {
            if (Node* parent = view->get_parent()) {
                for (int i = 0; i < parent->get_child_count(); i++) {
                    if (PhotonTransformView* trans_view = Object::cast_to<PhotonTransformView>(parent->get_child(i))) {
                        trans_view->receive_sync_data(pos, rot, scale);
                        break;
                    }
                }
            }
        }
        return;
    }

    if (eventCode == EVENT_INSTANTIATE) {
        String path = godot_dict["path"];
        int vid = godot_dict["vid"];
        int oid = godot_dict["oid"];
        Vector3 pos = godot_dict["pos"];
        Vector3 rot = godot_dict["rot"];

        Ref<PackedScene> scene = ResourceLoader::get_singleton()->load(path);
        if (scene.is_valid()) {
            Node* instance = scene->instantiate();
            if (PhotonView* view = _find_photon_view(instance)) {
                view->set_view_id(vid);
                view->set_owner_id(oid);
                if (Node3D* spatial = Object::cast_to<Node3D>(instance)) {
                    spatial->set_position(pos);
                    spatial->set_rotation(rot);
                } else if (Node2D* canvas = Object::cast_to<Node2D>(instance)) {
                    canvas->set_position(Vector2(pos.x, pos.y));
                    canvas->set_rotation(rot.z);
                }
                this->add_child(instance);
                UtilityFunctions::print("Photon: Instantiated remote object ", path, " with ViewID ", vid);
            } else {
                instance->queue_free();
            }
        }
        return;
    }

    if (eventCode == EVENT_DESTROY) {
        int vid = godot_dict["vid"];
        if (PhotonView* view = PhotonView::get_view(vid)) {
            if (Node* parent = view->get_parent()) {
                UtilityFunctions::print("Photon: Destroyed remote object with ViewID ", vid);
                parent->queue_free();
            }
        }
        return;
    }

    if (eventCode == EVENT_SET_PROPERTIES) {
        int type = godot_dict["t"];
        if (type == 0) {
            int pid = godot_dict["p"];
            String key = godot_dict["k"];
            Variant val = godot_dict["v"];
            if (!player_properties.has(pid)) player_properties[pid] = Dictionary();
            Dictionary props = player_properties[pid];
            props[key] = val;
            player_properties[pid] = props;
            emit_signal("player_property_changed", pid, key, val);
        } else if (type == 1) {
            String key = godot_dict["k"];
            Variant val = godot_dict["v"];
            room_properties[key] = val;
            emit_signal("room_property_changed", key, val);
        }
        return;
    }

    if (eventCode == EVENT_TRANSFER_OWNERSHIP) {
        int vid = godot_dict["vid"];
        int new_oid = godot_dict["oid"];
        if (PhotonView* view = PhotonView::get_view(vid)) {
            view->set_owner_id(new_oid);
            UtilityFunctions::print("Photon: ViewID ", vid, " ownership transferred to Player ", new_oid);
        }
        return;
    }

    if (eventCode == EVENT_RPC) {
        int vid = godot_dict["vid"];
        String method_name = godot_dict["m"];
        Array args = godot_dict["a"];
        if (PhotonView* view = PhotonView::get_view(vid)) {
            if (Node* target_node = view->get_parent()) {
                if (target_node->has_method(method_name)) {
                    target_node->callv(method_name, args);
                } else {
                    UtilityFunctions::printerr("Photon RPC Error: Method '", method_name, "' not found on node '", target_node->get_name(), "'!");
                }
            }
        }
        return;
    }

    if (eventCode == EVENT_LOAD_SCENE) {
        String scene_path = godot_dict["path"];
        UtilityFunctions::print("Photon: Network scene load requested: ", scene_path);
        if (is_inside_tree()) {
            TypedArray<Node> nodes = get_tree()->get_nodes_in_group(StringName("PhotonNetworked"));
            for (int i = 0; i < nodes.size(); i++) {
                Node* pv = Object::cast_to<Node>(nodes[i]);
                if (pv && pv->get_parent()) pv->get_parent()->queue_free();
            }
            get_tree()->change_scene_to_file(scene_path);
        }
        emit_signal("network_scene_loaded", scene_path);
        return;
    }

    if (eventCode == EVENT_RIGIDBODY_SYNC) {
        int vid = godot_dict["vid"];
        Variant pos = godot_dict.has("p") ? godot_dict["p"] : Variant();
        Variant rot = godot_dict.has("r") ? godot_dict["r"] : Variant();
        Variant lin_vel = godot_dict.has("lv") ? godot_dict["lv"] : Variant();
        Variant ang_vel = godot_dict.has("av") ? godot_dict["av"] : Variant();
        if (PhotonView* view = PhotonView::get_view(vid)) {
            if (Node* parent = view->get_parent()) {
                for (int i = 0; i < parent->get_child_count(); i++) {
                    if (PhotonRigidbodyView* rb_view = Object::cast_to<PhotonRigidbodyView>(parent->get_child(i))) {
                        rb_view->receive_sync_data(pos, rot, lin_vel, ang_vel);
                        break;
                    }
                }
            }
        }
        return;
    }

    if (eventCode == EVENT_ANIMATOR_SYNC) {
        int vid = godot_dict["vid"];
        Dictionary params = godot_dict["params"];
        
        if (PhotonView* view = PhotonView::get_view(vid)) {
            if (Node* parent = view->get_parent()) {
                for (int i = 0; i < parent->get_child_count(); i++) {
                    if (Node* anim_view = Object::cast_to<Node>(parent->get_child(i))) {
                        if (anim_view->get_class() == "PhotonAnimatorView") {
                            anim_view->call("receive_sync_data", params);
                            break;
                        }
                    }
                }
            }
        }
        return;
    }

    emit_signal("custom_event_received", playerNr, (int)eventCode, godot_dict);
}

// =====================================================================
// Listener Overrides
// =====================================================================

void PhotonClient::customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent) {
    ExitGames::Common::Hashtable evData = ExitGames::Common::ValueObject<ExitGames::Common::Hashtable>(eventContent).getDataCopy();
    Dictionary godot_dict = _hashtable_to_dict(evData);
    _process_custom_event(playerNr, (int)eventCode, godot_dict);
}

void PhotonClient::debugReturn(int debugLevel, const ExitGames::Common::JString& string) {}

void PhotonClient::connectionErrorReturn(int errorCode) {
    UtilityFunctions::printerr("PhotonClient: Connection Error Return! Code: ", errorCode);
    emit_signal("connection_error", errorCode);
}

void PhotonClient::clientErrorReturn(int errorCode) {
    UtilityFunctions::printerr("PhotonClient: Client Error Return! Code: ", errorCode);
}

void PhotonClient::warningReturn(int warningCode) {
    UtilityFunctions::print("PhotonClient: Warning Return! Code: ", warningCode);
}

void PhotonClient::serverErrorReturn(int errorCode) {
    UtilityFunctions::printerr("PhotonClient: Server Error Return! Code: ", errorCode);
}

void PhotonClient::connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster) {
    if (errorCode == 0) {
        UtilityFunctions::print("PhotonClient: Successfully connected to Master Server!");
        emit_signal("connected_to_master");
    } else {
        String err_msg(errorString.UTF8Representation().cstr());
        UtilityFunctions::printerr("PhotonClient: Connection failed! Code: ", errorCode, " Msg: ", err_msg);
        emit_signal("connection_error", errorCode);
    }
}

void PhotonClient::disconnectReturn() {
    UtilityFunctions::print("PhotonClient: Disconnected from server.");
    _cleanup_room_objects();
    emit_signal("disconnected");
}

void PhotonClient::joinLobbyReturn() {
    UtilityFunctions::print("PhotonClient: Successfully joined the Lobby!");
    emit_signal("lobby_joined");
}

void PhotonClient::leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString) {
    UtilityFunctions::print("PhotonClient: Left the room.");
    _cleanup_room_objects();
    emit_signal("room_left");
}

void PhotonClient::createRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString) {
    if (errorCode == 0) {
        String room_name = String(m_client->getCurrentlyJoinedRoom().getName().UTF8Representation().cstr());
        UtilityFunctions::print("PhotonClient: Room created successfully! Name: ", room_name);
        emit_signal("room_created", room_name);
    } else {
        String err_msg = String(errorString.UTF8Representation().cstr());
        UtilityFunctions::printerr("PhotonClient: Failed to create room! Code: ", errorCode, " Msg: ", err_msg);
        emit_signal("room_failed", errorCode, err_msg);
    }
}

void PhotonClient::joinRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString) {
    if (errorCode == 0) {
        String room_name = String(m_client->getCurrentlyJoinedRoom().getName().UTF8Representation().cstr());
        UtilityFunctions::print("PhotonClient: Joined room successfully! Name: ", room_name);
        emit_signal("room_joined", room_name);
    } else {
        String err_msg = String(errorString.UTF8Representation().cstr());
        UtilityFunctions::printerr("PhotonClient: Failed to join room! Code: ", errorCode, " Msg: ", err_msg);
        emit_signal("room_failed", errorCode, err_msg);
    }
}

void PhotonClient::joinRandomRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString) {
    if (errorCode == 0) {
        String room_name = String(m_client->getCurrentlyJoinedRoom().getName().UTF8Representation().cstr());
        UtilityFunctions::print("PhotonClient: Joined random room successfully! Name: ", room_name);
        emit_signal("room_joined", room_name);
    } else {
        String err_msg = String(errorString.UTF8Representation().cstr());
        UtilityFunctions::printerr("PhotonClient: Failed to join random room! Code: ", errorCode, " Msg: ", err_msg);
        emit_signal("room_failed", errorCode, err_msg);
    }
}

void PhotonClient::onRoomListUpdate() {
    if (!m_client) return;
    cached_room_list.clear();
    const ExitGames::Common::JVector<ExitGames::LoadBalancing::Room*>& rooms = m_client->getRoomList();
    
    for (unsigned int i = 0; i < rooms.getSize(); i++) {
        ExitGames::LoadBalancing::Room* room = rooms[i];
        if (!room || !room->getIsOpen()) continue;

        Dictionary room_info;
        String room_name = String(room->getName().UTF8Representation().cstr());
        room_info["name"] = room_name;
        room_info["player_count"] = room->getPlayerCount();
        room_info["max_players"] = room->getMaxPlayers();
        
        Dictionary custom_props = _hashtable_to_dict(room->getCustomProperties());
        Array keys = custom_props.keys();
        for (int j = 0; j < keys.size(); j++) {
            room_info[keys[j]] = custom_props[keys[j]];
        }
        cached_room_list[room_name] = room_info;
    }
    emit_signal("room_list_updated", cached_room_list);
}

void PhotonClient::joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const ExitGames::LoadBalancing::Player& player) {
    UtilityFunctions::print("PhotonClient: Player joined room! ID: ", playerNr);
    emit_signal("player_joined", playerNr);
}

void PhotonClient::leaveRoomEventAction(int playerNr, bool isInactive) {
    UtilityFunctions::print("PhotonClient: Player left room! ID: ", playerNr);
    if (is_inside_tree()) {
        StringName group_name = StringName("PhotonOwner_" + String::num_int64(playerNr));
        TypedArray<Node> nodes = get_tree()->get_nodes_in_group(group_name);
        for (int i = 0; i < nodes.size(); i++) {
            Node* pv = Object::cast_to<Node>(nodes[i]);
            if (pv && pv->get_parent()) {
                UtilityFunctions::print("Photon: Cleaning up abandoned object from Player ", playerNr);
                pv->get_parent()->queue_free();
            }
        }
    }
    player_properties.erase(playerNr);
    emit_signal("player_left", playerNr);
}

void PhotonClient::onMasterClientChanged(int clientId, int oldClientId) {
    UtilityFunctions::print("Photon: Master Client switched from Player ", oldClientId, " to Player ", clientId);
    emit_signal("master_client_switched", clientId, oldClientId);
}