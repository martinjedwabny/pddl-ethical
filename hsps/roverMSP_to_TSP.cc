
#include "soft.h"
#include "parser.h"
#include "preprocess.h"
#include "exec.h"
#include "cost_table.h"
#include "enumerators.h"

#include <sstream>
#include <fstream>

BEGIN_HSPS_NAMESPACE

StringTable symbols(50, lowercase_map);
Parser* reader = new Parser(symbols);

int main(int argc, char *argv[]) {

  for (int k = 1; k < argc; k++) {
    if ((strcmp(argv[k],"-v") == 0) && (k < argc - 1)) {
      Instance::default_trace_level = atoi(argv[++k]);
    }
    else if (*argv[k] != '-') {
      reader->read(argv[k], false);
    }
  }

  Statistics stats;
  stats.enable_interrupt(true);
  stats.start();

  reader->post_process();

  PDDL_Base::TypeSymbol* wp_type = reader->find_type("waypoint");
  assert(wp_type != 0);
  PDDL_Base::TypeSymbol* rover_type = reader->find_type("rover");
  assert(rover_type != 0);

  std::cout << "There are " << wp_type->elements.length() << " waypoints"
	    << std::endl;

  std::cout << "There are " << rover_type->elements.length() << " rovers"
	    << std::endl;

  PDDL_Base::symbol_vec a(0, 0);

  for (index_type k = 0; k < rover_type->elements.length(); k++) {
    weighted_graph rover_map(wp_type->elements.length());
    for (index_type i = 0; i < wp_type->elements.length(); i++)
      for (index_type j = 0; j < wp_type->elements.length(); j++)
	if (i != j) {
	  a.assign_value(0, 3);
	  a[0] = rover_type->elements[k];
	  a[1] = wp_type->elements[i];
	  a[2] = wp_type->elements[j];
	  if (reader->find_initial_fact("can_traverse", a)) {
	    NTYPE w = reader->find_function_value("traverse_cost", a);
	    rover_map.add_edge(i, j, w);
	  }
	}
    for (index_type i = 0; i < rover_map.size(); i++)
      rover_map.add_edge(i, i, 0);
    rover_map.transitive_closure();
    std::cout << "Travel cost matrix for rover #" << k << ":" << std::endl;
    rover_map.write_matrix(std::cout);
  }

  for (index_type k = 0; k < rover_type->elements.length(); k++) {
    a.assign_value(0, 2);
    a[0] = rover_type->elements[k];
    index_type p_init =
      reader->find_element_satisfying(wp_type->elements, "at", a, 1);
    if (p_init < wp_type->elements.length()) {
      std::cout << "Rover #" << k << " starts at waypoint #" << p_init
		<< std::endl;
    }
    else {
      std::cout << "Rover #" << k << " has no start position!"
		<< std::endl;
    }
  }

  PDDL_Base::TypeSymbol* lander_type = reader->find_type("lander");
  assert(lander_type != 0);
  index_set valid_end_wps;
  for (index_type k = 0; k < lander_type->elements.length(); k++) {
    a.assign_value(0, 2);
    a[0] = lander_type->elements[k];
    index_type lwp =
      reader->find_element_satisfying(wp_type->elements, "at_lander", a, 1);
    if (lwp != no_such_index) {
      assert(lwp < wp_type->elements.length());
      index_set can_see_lwp;
      a.assign_value(0, 2);
      a[1] = wp_type->elements[lwp];
      reader->find_elements_satisfying(wp_type->elements, "visible", a, 0,
				       can_see_lwp);
      valid_end_wps.insert(can_see_lwp);
    }
  }

  std::cout << "Valid ending waypoints for active rovers are "
	    << valid_end_wps << std::endl;

  index_set rovers_can_soil_data;
  a.assign_value(0, 1);
  reader->find_elements_satisfying(rover_type->elements,
				   "equipped_for_soil_analysis", a, 0,
				   rovers_can_soil_data);
  index_set rovers_can_rock_data;
  a.assign_value(0, 1);
  reader->find_elements_satisfying(rover_type->elements,
				   "equipped_for_rock_analysis", a, 0,
				   rovers_can_rock_data);

  PDDL_Base::TypeSymbol* cam_type = reader->find_type("camera");
  assert(cam_type != 0);
  PDDL_Base::TypeSymbol* obj_type = reader->find_type("objective");
  assert(obj_type != 0);
  index_vec cam_to_rover(no_such_index, cam_type->elements.length());
  index_set_vec cam_cal_wps(EMPTYSET, cam_type->elements.length());
  for (index_type k = 0; k < cam_type->elements.length(); k++) {
    // std::cerr << "checking camera " << k << "..." << std::endl;
    a.assign_value(0, 2);
    a[0] = cam_type->elements[k];
    cam_to_rover[k] =
      reader->find_element_satisfying(rover_type->elements, "on_board", a, 1);
    // std::cerr << "...it is on rover " << cam_to_rover[k] << std::endl;
    index_set cal_tgts;
    a.assign_value(0, 2);
    a[0] = cam_type->elements[k];
    reader->find_elements_satisfying(obj_type->elements,
				     "calibration_target", a, 1, cal_tgts);
    // std::cerr << "...calibration targets are " << cal_tgts << std::endl;
    for (index_type i = 0; i < cal_tgts.length(); i++) {
      a.assign_value(0, 2);
      a[0] = obj_type->elements[cal_tgts[i]];
      index_set cal_wps;
      reader->find_elements_satisfying(wp_type->elements,
				       "visible_from", a, 1, cal_wps);
      // std::cerr << "...waypoints for calibration target " << i << " are "
      //	<< cal_wps << std::endl;
      cam_cal_wps[k].insert(cal_wps);
    }
  }

  PDDL_Base::PredicateSymbol* p_have_soil_data =
    reader->find_predicate("communicated_soil_data");
  assert(p_have_soil_data != 0);

  PDDL_Base::PredicateSymbol* p_have_rock_data =
    reader->find_predicate("communicated_rock_data");
  assert(p_have_rock_data != 0);

  PDDL_Base::PredicateSymbol* p_have_image_data =
    reader->find_predicate("communicated_image_data");
  assert(p_have_image_data != 0);

  for (index_type k = 0; k < reader->dom_preferences.length(); k++) {
    PDDL_Base::atom_vec g(0, 0);
    assert(reader->goal_to_atom_vec(reader->dom_preferences[k]->goal, g));
    std::cout << reader->dom_preferences[k]->name->print_name << ": ";
    for (index_type i = 0; i < g.length(); i++) {
      if (i > 0) std::cout << "AND ";
      if (g[i]->pred == p_have_soil_data) {
	assert(g[i]->param.length() == 1);
	index_type g_wp = wp_type->elements.first(g[i]->param[0]);
	assert(g_wp != no_such_index);
	std::cout << "One of rovers " << rovers_can_soil_data
		  << " must visit waypoint #" << g_wp << std::endl;
      }
      else if (g[i]->pred == p_have_rock_data) {
	assert(g[i]->param.length() == 1);
	index_type g_wp = wp_type->elements.first(g[i]->param[0]);
	assert(g_wp != no_such_index);
	std::cout << "One of rovers " << rovers_can_rock_data
		  << " must visit waypoint #" << g_wp << std::endl;
      }
      else if (g[i]->pred == p_have_image_data) {
	assert(g[i]->param.length() == 2);
	PDDL_Base::Symbol* g_target = g[i]->param[0];
	PDDL_Base::Symbol* g_mode = g[i]->param[1];
	index_set g_cams;
	a.assign_value(0, 2);
	a[1] = g_mode;
	reader->find_elements_satisfying(cam_type->elements,
					 "supports", a, 0, g_cams);
	index_set g_rovers(g_cams, cam_to_rover);
	index_set g_wps;
	a.assign_value(0, 2);
	a[0] = g_target;
	reader->find_elements_satisfying(wp_type->elements,
					 "visible_from", a, 1, g_wps);
	for (index_type j = 0; j < g_cams.length(); j++) {
	  if (j > 0) std::cout << " OR ";
	  std::cout << "Rover " << cam_to_rover[g_cams[j]]
		    << " must visit one of waypoints " << g_wps
		    << " and must visit one of waypoints "
		    << cam_cal_wps[g_cams[j]]
		    << " before that"
		    << std::endl;
	}
      }
      else {
	std::cout << "error: don't know how to treat goal "
		  << reader->dom_preferences[k]->name->print_name
		  << std::endl;
      }
    }
  }

  stats.stop();
}

END_HSPS_NAMESPACE

#ifdef USE_HSPS_NAMESPACE

int main(int argc, char *argv[])
{
  return HSPS::main(argc, argv);
}

#endif
