#include "fsltl.hh"
#include "tgbaIts.hh"
#include "Composite.hh"

namespace its {


  // Create a type to hold a spot TGBA
  bool fsltlModel::declareType (const class spot::tgba * tgba) {
    TgbaType * newtgba = new TgbaType (tgba);
    // store reference for later use
    tgba_ = newtgba;
    // Compute and store the AP set to update the sogIts component
    // ??? how to is in the sogtgbautils file.
    return addType(newtgba);
  }

  
  // Build the composed system based on the TGBA and the main instance of the ITSModel.
  // Precondition : we have built a model, invoked setInstance and setInstanceState, and declared a TGBA type.
  // Side-effect : updates the main instance
  // Returns false and abort if any preconditions are unfulfilled
  bool fsltlModel::buildComposedSystem () {
    // instance not set
    if (getInstance() == NULL) {
      std::cerr << "Main instance state not set when calling \"buildComposedSystem\"\n" ;
      return false;
    }
    // init state unset
    if (getInitialState() == State::null) {
      std::cerr << "Initial state not set when calling \"buildComposedSystem\"\n" ;
      return false;
    }
    // lookup the TGBA
    pType tgba = findType("TGBA");
    if (! tgba) {
      std::cerr << "TGBA not set when calling \"buildComposedSystem\"\n" ;
      return false;
    }

    std::cout << *this << std::endl;

    // Ok preconditions fulfilled.
    // Build the composite.
    vLabel compname = tgba->getName() + "x" + getInstance()->getType()->getName();
    Composite * comp = new Composite(compname);
    // Instance names
    vLabel syst = "system";
    vLabel aut = "tgba";
    // an instance of the system
    comp->addInstance (syst, getInstance()->getType()->getName(), *this);
    // an instance of the TGBA
    comp->addInstance (aut, tgba->getName(), *this);

    // Grab the TGBA labels
    labels_t tgbalabs = tgba->getTransLabels();

    for (labels_it it = tgbalabs.begin() ; it != tgbalabs.end() ; ++it ) {
      std::cerr << "label :" << *it << std::endl;
      TgbaType::tgba_arc_label_t arcLab = tgba_->getTransLabelDescription(*it);
      std::cerr << "desc :" << arcLab.first << ":" << arcLab.second << std::endl;      
      Transition apcond = sogIts_->getSelector(arcLab.first, getInstance()->getType());

      labels_t labtodo;
      labtodo.push_back(*it);
      Transition toadd = localApply(tgba->getSuccs(labtodo),0) & localApply(getInstance()->getType()->getLocals()  & apcond, 1) ;
      allTrans_ = allTrans_ + toadd;
      if (arcLab.second != bddfalse) {
	accToTrans_it accit = accToTrans_.find(arcLab.second.id());
	if (accit == accToTrans_.end()) {
	  // first occurrence
	  
	  accToTrans_ [arcLab.second.id()] = toadd;
	} else {
	  accit->second = accit->second + toadd;
	}
      }
      
    }

    std::cout << "Built model with initstate :" << getInitState() << std::endl;
    std::cout << "Transition rel (all) :" << allTrans_ << std::endl;


    return true;
  }

  State fsltlModel::getInitState () {
    return State(1, getInstance()->getType()->getState("init"), State(0, findType("TGBA")->getState("init")));
  }

  Transition fsltlModel::getNextByAll () {
    return allTrans_;
  }

  Transition fsltlModel::getNextByAcc (bdd acc) {
    return Transition::id;
  }  

  State fsltlModel::findSCC () {
    State reach = fixpoint (getNextByAll() + Transition::id) ( getInitState()); 
    
    std::cout << "Reachable states : " << reach.nbStates();
    if (reach.nbStates() < 15)
      std::cout << reach << std::endl;

    State div = fixpoint (getNextByAll()) (reach);

    std::cout << "Divergence ? " << div.nbStates() << std::endl;
    if (div.nbStates() < 15)
      std::cout << div << std::endl;
    return State::null;
  }


}

