^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package op_utilities
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1.9.0 (2018-10-31)
------------------
* [fix] PascalCase messages (`#1408 <https://github.com/kfunaoka/Autoware/issues/1408>`_)
  * Switch message files to pascal case
  * Switch message names to pascal case in Runtime Manager
  * Switch message names to pascal case in *.yaml
  * Rename brake_cmd and steer_cmd to BrakeCmd and SteerCmd in main.yaml
* Feature/beyond pixel tracker (`#1473 <https://github.com/kfunaoka/Autoware/issues/1473>`_)
  * Add beyond_pixel node
  * Update prototype of beyond pixel (`#1430 <https://github.com/kfunaoka/Autoware/issues/1430>`_)
  * Add parser of DetectedObjectArray for beyond tracker(`#1430 <https://github.com/kfunaoka/Autoware/issues/1430>`_)
  * * Adaptations to the original code
  * Added README
  * Added Runtime Manager entry
  * Added Video link
  * Added install commands for cmake
  * * Add ID only to tracked objects
  * Display valid IDs on the 3D labels
  * Display only objects with image coords
  * * Added Minimum dimensions
  * Register angle from the vision tracker if available
  * Keep message publishing rate continuous
  * Revert platform_automation_msgs (`#1498 <https://github.com/kfunaoka/Autoware/issues/1498>`_)
  * Code cleanup
  * Fixed a crash when the dimensions are outside of the image
  * Fix annoying catkin_make causing to run twice the Cmake generation
* Contributors: Abraham Monrroy, Esteve Fernandez

1.8.0 (2018-08-31)
------------------
* Support old behavior of insert static object for obstacle avoidance testing
  Only one simulated car available in the runtime manager
  update for copywrite note
  insert autoware_build_flags to new nodes
* Update README.md
* Add README files for OpenPlanner packages
* Modify Map loading for OpenPlanner, now it reads from Autoware vector map messages, old behavior still works but from launch file only.
  Delete way_planner, dp_planner from UI, but they still accessible from roslaunch.
* Update op_utility files for csv files loading
  Update MappingHelpers with latest modifications
  Update PlanningHelpers with latest modifications
  add op_common_param node, for setting OpenPlanner parameter for all related nodes such as lidar_kf_contour_track
  Improve tracking by including size different in association function
  Update way_planner, dp_planner for compatibility with new Mapping Modifications, Map format is backward compatible
* Update OpenPlanner libraries (op_planner, op_utitity, op_ros_helpers)
  Update ring ground filter with latest implementation
  Update lidar_kf_contour_track with latest implementation
  Add op_utilities nodes (op_bag_player, op_data_logger, op_pose2tf)
  Modify autoware_msgs for OpenPlanner use (CloudCluster, DetectedObject, lane, waypoint)
  Update UI computing.yaml for the new nodes and modifies parameters
  Update UI sensing.yaml for updated ring_ground_filter params
* Contributors: Hatem Darweesh, hatem-darweesh
