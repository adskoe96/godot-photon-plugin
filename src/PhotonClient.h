#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "LoadBalancing-cpp/inc/Client.h"
#include "LoadBalancing-cpp/inc/Listener.h"

namespace godot {

class PhotonClient : public Node, public ExitGames::LoadBalancing::Listener {
    GDCLASS(PhotonClient, Node)

public:
    enum ClientState {
        STATE_CONNECTING_TO_NAMESERVER = 2,
        STATE_CONNECTED_TO_NAMESERVER = 3,
        STATE_DISCONNECTING_FROM_NAMESERVER = 4,
        STATE_CONNECTING_TO_MASTERSERVER = 5,
        STATE_CONNECTED_TO_MASTERSERVER = 8,
        STATE_JOINED_LOBBY = 9
    };

    enum EventCode {
        EVENT_TRANSFORM_SYNC = 201,
        EVENT_INSTANTIATE = 202,
        EVENT_DESTROY = 203,
        EVENT_SET_PROPERTIES = 204,
        EVENT_TRANSFER_OWNERSHIP = 205,
        EVENT_RPC = 206,
        EVENT_LOAD_SCENE = 207,
        EVENT_RIGIDBODY_SYNC = 208,
        EVENT_ANIMATOR_SYNC = 209
    };

private:
    String current_app_id;
    String current_app_version;
    ExitGames::LoadBalancing::Client* m_client;
    bool offline_mode;
    int instantiated_count = 0;

    Dictionary room_properties;
    Dictionary player_properties;
    Dictionary cached_room_list;

    ExitGames::Common::Hashtable _dict_to_hashtable(const Dictionary& dict);
    Dictionary _hashtable_to_dict(const ExitGames::Common::Hashtable& table);
    class PhotonView* _find_photon_view(class Node* parent);
    void _process_custom_event(int playerNr, int eventCode, const Dictionary& godot_dict);
    void _cleanup_room_objects();

protected:
    static void _bind_methods();

public:
    PhotonClient();
    ~PhotonClient();

    static PhotonClient* get_singleton();

    void _process(double delta) override;

    void set_offline_mode(bool offline);
    bool get_offline_mode() const;

    void connect_to_server(const String& p_app_id = "", const String& p_app_version = "", const String& p_region = "eu");

    bool join_lobby();
    int get_state();
    int get_ping();

    void create_room(const String& room_name, int max_players = 4, bool is_open = true, bool is_visible = true, const Dictionary& custom_properties = Dictionary());
    void join_room(const String& room_name);
    void join_random_room();
    void join_or_create_room(const String& room_name);
    void leave_room();
    Dictionary get_room_list();

    int get_local_player_id();
    bool is_master_client();
    bool set_master_client(int player_id);
    int get_master_client_id();
    Dictionary get_player_list();

    class Node* instantiate(const String& prefab_path, const Vector3& position, const Vector3& rotation);
    void destroy(class Node* target_node);
    void load_network_scene(const String& scene_path);
    void raise_custom_event(int event_code, const Dictionary& data, int rpc_target = 1, bool reliable = true);

    void set_player_property(const String& key, const Variant& value);
    Variant get_player_property(int player_id, const String& key);
    
    void set_room_property(const String& key, const Variant& value);
    Variant get_room_property(const String& key);

    // =====================================================================
    // ExitGames::LoadBalancing::Listener Overrides
    // =====================================================================
    void debugReturn(int debugLevel, const ExitGames::Common::JString& string) override;
    void connectionErrorReturn(int errorCode) override;
    void clientErrorReturn(int errorCode) override;
    void warningReturn(int warningCode) override;
    void serverErrorReturn(int errorCode) override;
    void joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const ExitGames::LoadBalancing::Player& player) override;
    void leaveRoomEventAction(int playerNr, bool isInactive) override;
    void customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent) override;
    void connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster) override;
    void disconnectReturn() override;
    void joinLobbyReturn() override;
    void leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString) override;
    void createRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString) override;
    void joinRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString) override;
    void joinRandomRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString) override;
    void onRoomListUpdate() override;
    void onMasterClientChanged(int clientId, int oldClientId) override;
};

} // namespace godot

VARIANT_ENUM_CAST(godot::PhotonClient::ClientState);
VARIANT_ENUM_CAST(godot::PhotonClient::EventCode);