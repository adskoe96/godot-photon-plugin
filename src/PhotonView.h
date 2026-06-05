#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/templates/hash_map.hpp>

namespace godot {

class PhotonView : public Node {
    GDCLASS(PhotonView, Node)

public:
    enum RpcTarget {
        RPC_ALL = 0,
        RPC_OTHERS = 1,
        RPC_MASTER_CLIENT = 2,
        RPC_ALL_BUFFERED = 3,
        RPC_OTHERS_BUFFERED = 4
    };

private:
    int view_id;
    int owner_id;
    static HashMap<int, PhotonView*> view_registry;

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    PhotonView();
    ~PhotonView();

    void set_view_id(int p_id);
    int get_view_id() const;

    void set_owner_id(int p_id);
    int get_owner_id() const;

    bool is_mine() const;
    void transfer_ownership(int new_player_id);
    void photon_rpc(const String& method_name, const Array& args, int target = RPC_OTHERS);

    static PhotonView* get_view(int p_view_id);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::PhotonView::RpcTarget);