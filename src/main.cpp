// Copyright (C) 2004, 2009, 2010, 2011 Laboratoire d'Informatique de Paris
// 6 (LIP6), d�partement Syst�mes R�partis Coop�ratifs (SRC),
// Universit� Pierre et Marie Curie.
//
// This file is part of the Spot tutorial. Spot is a model checking
// library.
//
// Spot is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Spot is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Spot; see the file COPYING.  If not, write to the Free
// Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#include <iostream>
#include <fstream>
#include <list>
#include <sstream>
#include <string>
#include <cstring>

#include "bdd.h"
#include "ltlparse/public.hh"
#include "ltlvisit/destroy.hh"

#include "sogtgbautils.hh"
#include "train.hh"
#include "MemoryManager.h"

// prod parser
#include "Options.hh"

// fair CTL bricks
#include "tgbaIts.hh"
#include "fsltl.hh"

using namespace its;
using namespace sogits;
using std::cerr;
using std::string;
using std::endl;


void usage() {
  cerr << "Instantiable Transition Systems SDD/DDD LTL Analyzer;" <<endl;
  cerr << "Mandatory options : -i -t to provide input model, -ltl or -LTL to provide formulae" << std::endl;

  usageInputOptions();
  usageSDDOptions();

  cerr << "This tool performs LTL verification on state-space of ITS" <<endl;
  cerr << " LTL specific options for  package " << PACKAGE_STRING << endl;

  std::cerr << "  MANDATORY : specify a formula to check " << std::endl
	    << "  -ltl formula       specify the ltl formula. Must be stuttering invariant for SOG and SOP variants." << std::endl
	    << "Actions:" << std::endl
            << "  -aALGO          apply the emptiness check algoritm ALGO"
            << std::endl
            << "  -SSOGTYPE       apply the SOG construction algoritm SOGTYPE={SOG,SLAP,SOP,FSOWCTY,FSEL,BCZ99,SLAP-FST,SLAP-FSA} (SLAP-FST by default)\n"
	    << "                  The FST variants include a test for switching to fully symbolic emptiness check in terminal states.\n"
	    << "                  The FSA variants include a test for switching to fully symbolic emptiness check in any potentially accepting automaton state."
            << std::endl
            << "  --place-syntax           suppose that atomic properties are just names of variables: \"Idle\" will be interpreted as \"Idle=1\""
            << std::endl
            << "  -C              display the number of states and edges of the SOG"
            << std::endl
            << "  -c              check the formula" << std::endl
            << "  -e              display a sequence (if any) of the net "
            << "satisfying the formula (implies -c)" << std::endl

    //            << "  -LTL formula_file  formula read from formula_file"
    //        << std::endl
            << "  -g              display the sog"
            << std::endl
            << "  -p              display the net"
            << std::endl
            << "  -s              show the formula automaton"
            << std::endl
            << "Options of the formula transformation:"
            << std::endl
	    << "  -dR3            disable the SCC reduction" << std::endl
	    << "  -R3f            enable full SCC reduction" << std::endl
            << "  -b              branching postponement"
            << " (false by default)" << std::endl
            << "  -l              fair-loop approximation"
            << " (false by default)" << std::endl
            << "  -x              try to produce a more deterministic automaton"
            << " (false by default)" << std::endl
            << "  -y              do not merge states with same symbolic "
            << "representation (true by default)"
            << std::endl
            << "Where ALGO should be one of:" << std::endl
            << "  Cou99(OPTIONS) (the default)" << std::endl
            << "  CVWY90(OPTIONS)" << std::endl
            << "  GV04(OPTIONS)" << std::endl
            << "  SE05(OPTIONS)" << std::endl
            << "  Tau03(OPTIONS)" << std::endl
            << "  Tau03_opt(OPTIONS)" << std::endl;
  exit(2);
}

int main(int argc, const char *argv[]) {

  // external block for full garbage
  {



  bool check = false;
  bool print_rg = false;
  bool print_pn = false;
  bool count = false;

  bool ce_expected = false;
  bool fm_exprop_opt = false;
  bool fm_symb_merge_opt = true;
  bool post_branching = false;
  bool fair_loop_approx = false;
  bool print_formula_tgba = false;

  bool scc_optim = true;
  bool scc_optim_full = false;

  std::string ltl_string = "1"; // true
  std::string algo_string = "Cou99";

  sog_product_type sogtype = SLAP_FST;

  bool isPlaceSyntax = false;

  // echo options of run
  std::cout << "its-ltl command run as :\n" << std::endl;
  for (int i=0;i < argc; i++) {
    std::cout << argv[i] << "  ";
  }
  std::cout << std::endl;

  // Build the options vector
  std::vector<const char *> args;
  for (int i=1;i < argc; i++) {
    args.push_back(argv[i]);
  }

  std::vector<const char *> argsleft;
  argc = args.size();

  for (int i=0;i < argc; i++) {
    if (!strncmp(args[i], "-a", 2)) {
      algo_string = args[i]+2;
    }
    else if (!strcmp(args[i], "-b")) {
      post_branching = true;
    }
    else if (!strcmp(args[i], "-c")) {
      check = true;
    }
    else if (!strcmp(args[i], "-C")) {
      count = true;
    }
    else if (!strcmp(args[i], "-e")) {
      ce_expected = true;
    }
    else if (!strcmp(args[i], "-s")) {
      print_formula_tgba = true;
    }
    else if (!strncmp(args[i], "-ltl", 4)) {
      if (++i > argc)
	{ cerr << "give argument value for ltl formula please after " << args[i-1]<<endl; usage() ; exit(1);}
      ltl_string = args[i];
    }
    else if (!strncmp(args[i], "-dR3", 4)) {
      scc_optim = false;
    }
    else if (!strncmp(args[i], "-R3f", 4)) {
      scc_optim = true;
      scc_optim_full = true;
    }
    else if (!strcmp(args[i], "--place-syntax")) {
      isPlaceSyntax = true;
    }

//     else if (!strncmp(args[i], "-ltl", 4)) {
//       std::ifstream fin(args[i]+2);
//       if (!fin) {
//           std::cerr << "Cannot open " << args[i]+2 << std::endl;
//           exit(2);
//       }
//       if (!std::getline(fin, ltl_string, '\0')) {
//           std::cerr << "Cannot read " << args[i]+2 << std::endl;
//           exit(2);
//       }
//     }
    else if (!strcmp(args[i], "-g")) {
      print_rg = true;
    }
    else if (!strcmp(args[i], "-l")) {
      fair_loop_approx = true;
    }
    else if (!strcmp(args[i], "-p")) {
      print_pn = true;
    }
    else if (!strcmp(args[i], "-SSOG")) {
      sogtype = PLAIN_SOG;
    }
    else if (!strcmp(args[i], "-SBCZ99")) {
      sogtype = BCZ99;
    }
    else if (!strcmp(args[i], "-SSLAP")) {
      sogtype = SLAP_NOFS;
    }
    else if (!strcmp(args[i], "-SSLAP-FSA")) {
      sogtype = SLAP_FSA;
    }
    else if (!strcmp(args[i], "-SSLAP-FST")) {
      sogtype = SLAP_FST;
    }
    else if (!strcmp(args[i], "-SSOP")) {
      sogtype = SOP;
    }
    else if (!strcmp(args[i], "-SFSEL")) {
      sogtype = FS_EL;
    }
    else if (!strcmp(args[i], "-SFSOWCTY")) {
      sogtype = FS_OWCTY;
    }
    else if (!strcmp(args[i], "-x")) {
      fm_exprop_opt = true;
    }
    else if (!strcmp(args[i], "-y")) {
      fm_symb_merge_opt = false;
    }
    else {
      argsleft.push_back(args[i]);
    }
  }
  args = argsleft;

  ITSModel * model;
  if (sogtype == FS_OWCTY || sogtype == FS_EL) {
    model = new fsltlModel();
  } else {
    model = new ITSModel();
  }

    // parse command line args to get the options
  if (! handleInputOptions (args, *model) ) {
    usage();
    return 1;
  }
  // we now should have the model defined.
  string modelName = model->getInstance()->getType()->getName();

  bool with_garbage = true;
  // Setup SDD specific settings
  if (!handleSDDOptions (args, with_garbage)) {
    usage();
    return 1;
  }

  if (! args.empty()) {
    std::cerr << "Unrecognized command line arguments :" ;
    for (size_t i = 0; i < args.size(); ++i) {
      std::cerr << args[i] << " ";
    }
    std::cerr << std::endl;
    usage();
    return 1;
  }

  if (print_pn)
    std::cout << *model << std::endl;


//   // Parse and build the model !!!
//   loadTrains(2,model);
//   // Update the model to point at this model type as main instance
//   model.setInstance("Trains","main");
//   // The only state defined in the type "trains" is "init"
//   // This sets the initial state of the main instance
//   model.setInstanceState("init");

  // Initialize spot
  spot::ltl::parse_error_list pel;

  spot::ltl::formula* f = spot::ltl::parse(ltl_string, pel);
  if (spot::ltl::format_parse_errors(std::cerr, ltl_string, pel)) {
    f->destroy();
    return 1;
  }

  if (check) {
    LTLChecker checker;
    checker.setFormula(f);
    checker.setModel(model);
    checker.setOptions(algo_string, ce_expected,
		       fm_exprop_opt, fm_symb_merge_opt,
		       post_branching, fair_loop_approx, "STATS", print_rg,
		       scc_optim, scc_optim_full, print_formula_tgba);
    if (isPlaceSyntax) {
      checker.setPlaceSyntax(true);
    }
    checker.model_check(sogtype);
  }

  f->destroy();
  delete model;
  // external block for full garbage
  }
  MemoryManager::garbage();

  return 0;

}
