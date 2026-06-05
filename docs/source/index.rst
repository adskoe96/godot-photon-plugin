===================================================
Photon GDExtension documentation
===================================================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

Introduction
============

This plugin provides an integration of the Photon client (ExitGames LoadBalancing API) for the Godot engine using GDExtension (C++). The plugin includes a system for object synchronization, remote procedure calls (RPC), and room management.

Project Setup
=============

Before using the plugin, you need to configure the Project Settings:

1. Go to **Project -> Project Settings**.
2. In the settings section, locate the plugin configuration (they will appear after activation or can be added manually):
   - ``addons/photon/connection/app_id``: Your unique App ID from the Photon dashboard.
   - ``addons/photon/connection/app_version``: The version of your application (default is "1.0").

API Reference
=============

.. class:: PhotonClient

   The main singleton for managing network connections, rooms, and object instantiation.

   .. property:: bool offline_mode

      When enabled, allows you to test the game without actually connecting to the Photon servers.

   .. method:: void connect_to_server(String app_id = "", String app_version = "", String region = "eu")

      Initiates a connection to the Photon servers. If ``app_id`` is not provided, it uses the value from the project settings.

   .. method:: void create_room(String room_name, int max_players = 4, bool is_open = true, bool is_visible = true, Dictionary custom_properties = Dictionary())

      Creates a new game room with the specified parameters and custom properties.

   .. method:: void join_room(String room_name)
               void join_random_room()
               void join_or_create_room(String room_name)

      A group of methods for joining existing rooms or creating them automatically.

   .. method:: Node* instantiate(String prefab_path, Vector3 position = Vector3(), Vector3 rotation = Vector3())

      Creates a local copy of the scene located at ``prefab_path`` and broadcasts a network event to instantiate the object for other clients. The scene must contain a ``PhotonView`` node.

   .. method:: void destroy(Node* target_node)

      Destroys the networked object locally and for all other clients. The object must belong to the current client (either as the Master Client or the owner).

   .. method:: void raise_custom_event(int event_code, Dictionary data, int rpc_target = 1, bool reliable = true)

      Sends a custom event over the network.

   **Signals:**

   * ``connected_to_master()``
   * ``disconnected()``
   * ``room_joined(String room_name)``
   * ``player_joined(int player_id)``
   * ``custom_event_received(int player_id, int event_code, Dictionary data)``

     Core signals for tracking network state and player actions.

.. class:: PhotonView

   The base node for network identity. Required for every object whose state needs to be synchronized or which needs to receive RPCs.

   .. property:: int view_id
      
      The unique network identifier of the object.

   .. property:: int owner_id

      The ID of the client that owns this object.

   .. method:: bool is_mine()

      Returns ``true`` if the local client is the owner of this object. Used to separate input/control logic.

   .. method:: void photon_rpc(String method_name, Array args, int target = RPC_OTHERS)

      Calls a method named ``method_name`` on all clients (according to the ``target``).

.. class:: PhotonTransformView

   A node for automatically synchronizing transformations (Node2D and Node3D).

   .. property:: bool sync_position
   .. property:: bool sync_rotation
   .. property:: bool sync_scale

      Flags that determine exactly which transformation components will be sent over the network.

   .. property:: float sync_rate
      
      The frequency of network updates (default is 20.0 times per second).

   .. property:: float lerp_speed

      The interpolation speed between the current and target (networked) position.

.. class:: PhotonRigidbodyView

   A node for synchronizing physics objects (RigidBody2D and RigidBody3D).

   .. property:: bool sync_linear_velocity
   .. property:: bool sync_angular_velocity

      Allows synchronization of physical forces acting on the object, in addition to its position and rotation.

.. class:: PhotonAnimatorView

   A node for synchronizing AnimationTree parameters over the network.

   .. property:: NodePath anim_tree_path

      The path to the AnimationTree node.

   .. property:: Array sync_parameters

      An array of strings (names of AnimationTree parameters) that need to be transmitted over the network (e.g., `"parameters/BlendSpace2D/blend_position"`).