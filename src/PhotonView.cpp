#include "PhotonView.h"
#include "PhotonClient.h"
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    HashMap<int, PhotonView*> PhotonView::view_registry;
}

using namespace godot;

void PhotonView::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_view_id", "id"), &PhotonView::set_view_id);
    ClassDB::bind_method(D_METHOD("get_view_id"), &PhotonView::get_view_id);
    ClassDB::bind_method(D_METHOD("set_owner_id", "id"), &PhotonView::set_owner_id);
    ClassDB::bind_method(D_METHOD("get_owner_id"), &PhotonView::get_owner_id);
    ClassDB::bind_method(D_METHOD("is_mine"), &PhotonView::is_mine);
    ClassDB::bind_method(D_METHOD("transfer_ownership", "new_player_id"), &PhotonView::transfer_ownership);
    ClassDB::bind_method(D_METHOD("photon_rpc", "method_name", "args", "target"), &PhotonView::photon_rpc, DEFVAL(RPC_OTHERS));

    ADD_PROPERTY(PropertyInfo(Variant::INT, "view_id"), "set_view_id", "get_view_id");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "owner_id"), "set_owner_id", "get_owner_id");

    BIND_ENUM_CONSTANT(RPC_ALL);
    BIND_ENUM_CONSTANT(RPC_OTHERS);
    BIND_ENUM_CONSTANT(RPC_MASTER_CLIENT);
    BIND_ENUM_CONSTANT(RPC_ALL_BUFFERED);
    BIND_ENUM_CONSTANT(RPC_OTHERS_BUFFERED);
}

PhotonView::PhotonView() {
    view_id = 0;
    owner_id = 0;
}

PhotonView::~PhotonView() {}

void PhotonView::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE:
            if (view_id != 0) view_registry[view_id] = this;
            if (owner_id != 0) add_to_group(StringName("PhotonOwner_" + String::num_int64(owner_id)));
            add_to_group(StringName("PhotonNetworked")); 
            break;
            
        case NOTIFICATION_EXIT_TREE:
            if (view_id != 0) view_registry.erase(view_id);
            if (owner_id != 0) remove_from_group(StringName("PhotonOwner_" + String::num_int64(owner_id)));
            remove_from_group(StringName("PhotonNetworked"));
            break;
    }
}

void PhotonView::set_view_id(int p_id) { view_id = p_id; }
int PhotonView::get_view_id() const { return view_id; }

void PhotonView::set_owner_id(int p_id) { 
    if (owner_id != 0 && is_inside_tree()) {
        remove_from_group(StringName("PhotonOwner_" + String::num_int64(owner_id)));
    }
    owner_id = p_id; 
    if (owner_id != 0 && is_inside_tree()) {
        add_to_group(StringName("PhotonOwner_" + String::num_int64(owner_id)));
    }
}

int PhotonView::get_owner_id() const { return owner_id; }

bool PhotonView::is_mine() const {
    PhotonClient* client = PhotonClient::get_singleton();
    if (client && client->get_local_player_id() != 0) {
        return owner_id == client->get_local_player_id();
    }
    return true; 
}

void PhotonView::transfer_ownership(int new_player_id) {
    if (owner_id == new_player_id) return;
    int old_owner = owner_id;
    set_owner_id(new_player_id);

    PhotonClient* client = PhotonClient::get_singleton();
    if (client) {
        Dictionary data;
        data["vid"] = view_id;
        data["oid"] = new_player_id;
        client->raise_custom_event(PhotonClient::EVENT_TRANSFER_OWNERSHIP, data, PhotonView::RPC_OTHERS_BUFFERED);
    }
    UtilityFunctions::print("Photon: Transferred ownership of ViewID ", view_id, " from ", old_owner, " to ", new_player_id);
}

void PhotonView::photon_rpc(const String& method_name, const Array& args, int target) {
    PhotonClient* client = PhotonClient::get_singleton();
    if (!client) return;

    Dictionary data;
    data["vid"] = view_id;
    data["m"] = method_name;
    data["a"] = args;
    client->raise_custom_event(PhotonClient::EVENT_RPC, data, target);
}

PhotonView* PhotonView::get_view(int p_view_id) {
    if (view_registry.has(p_view_id)) return view_registry[p_view_id];
    return nullptr;
}