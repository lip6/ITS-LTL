#ifndef SOG_TGBA_UTILS_HH
#define SOG_TGBA_UTILS_HH

#include <set>
#include <list>
#include <string>

#include "misc/timer.hh"
#include "ltlast/formula.hh"
#include "ltlast/atomic_prop.hh"
#include "ltlvisit/apcollect.hh"
#include "tgba/bdddict.hh"

#include "ITSModel.hh"

#include "sogtgba.hh"

namespace sogits {

/// \brief Display some statistics concerning the symbolic observation
/// graph constructed for \a n when considering the observable
/// transitions \a obs_tr.
///
/// \dontinclude countmarkings.cpp
/// \skipline void count_markings
/// \until } //
  void count_markings(const its::ITSModel & m,  const spot::ltl::formula* f);

/// \brief Display the symbolic observation graph constructed for \a n
/// when considering the observable transitions \a obs_tr.
///
/// \dontinclude printrg.cpp
/// \skipline void print_reachability_graph
/// \until } //
  void print_reachability_graph(const its::ITSModel & m, const spot::ltl::formula* f);

/// \brief Check if the atomic propositions in \a f are transitions of the Petri
/// net \a p. Return a pointer on the first atomic proposition which is not
/// a transition name if any and 0 otherwise.
///
/// \dontinclude modelcheck.cpp
/// \skipline const std::string* check_at_prop
/// \until } //
  std::string* check_at_prop(its::ITSModel & m,
                           const spot::ltl::formula* f,
                           spot::ltl::atomic_prop_set*& sap);



  enum sog_product_type {
    FSLTL, // < The fully symbolic approach: encode tgba as an ITS.
    PLAIN_SOG, // < The basic algorithm with static parameters.
    SLAP_NOFS, // < The (Symbolic) Local Obs Graph algorithm
    SLAP_FST, // < The (Symbolic) Local Obs Graph algorithm with Terminal states FSLTL test.
    SLAP_FSA, // < The (Symbolic) Local Obs Graph algorithm with Accepting states FSLTL test.
    DSOG // < The Dynamic Symbolic Obs Graph algorithm
  } ;



  /// An enum to capture the strategies related to activation of Fully-Symbolic emptiness check.
  enum FSTYPE {
    NOFS, // < Do not use FS emptiness check at all
    FST, // < Use FS emptiness check, but only for terminal states (no successor except self, a priori => all acceptance conditions represented)
    FSA // < Use FS emptiness check for all states potentially accepting (self loop acceptance conds = full acceptance cond set)
  };



  class LTLChecker {

    spot::ltl::atomic_prop_set *sap_;
    const spot::ltl::formula* f_;

    // For bdd varnum to AP name in Spot
    spot::bdd_dict dict_;

    its::ITSModel * model_;
    sogIts * sogModel_;

    const spot::tgba* a_;
    sog_tgba * systgba_;
    const spot::tgba* tba_;
    // options
    std::string echeck_algo_;
    bool ce_expected_;
    bool fm_exprop_opt_;
    bool fm_symb_merge_opt_;
    bool post_branching_;
    bool fair_loop_approx_;
    std::string ltl_string_;
    bool display_;
    bool scc_optim_;
    bool scc_optim_full_;
    bool print_formula_tgba_;

    spot::timer_map timers;

    bool buildTgbaFromformula ();

    void fs_model_check();
  public :
    LTLChecker () : sap_(NULL),
		    f_(NULL),
		    dict_(),
		    model_(NULL),
		    sogModel_(NULL),
		    a_(NULL),
		    systgba_(NULL),
		    tba_(NULL),
		    fm_exprop_opt_(false),
		    fm_symb_merge_opt_(true),
		    post_branching_(false),
		    fair_loop_approx_(false),
		    ltl_string_("formula"),
		    display_(false),
		    scc_optim_(true),
		    scc_optim_full_(false),
                    print_formula_tgba_(false) {}

    ~LTLChecker();

    void setFormula (const spot::ltl::formula* f) {
      f_ = f;
    }

    void setModel (its::ITSModel * model) {
      model_ = model;
    }

    void setOptions (const std::string& echeck_algo,
		     bool ce_expected,
		     bool fm_exprop_opt=false,
		     bool fm_symb_merge_opt=true,
		     bool post_branching=false,
		     bool fair_loop_approx=false,
		     const std::string & ltl_string ="formula",
		     bool dotdump = false,
		     bool scc_optim = true,
		     bool scc_optim_full = false,
		     bool print_formula_tgba = false)
    {
      echeck_algo_ = echeck_algo;
      ce_expected_ = ce_expected;
      fm_exprop_opt_ = fm_exprop_opt;
      fm_symb_merge_opt_ = fm_symb_merge_opt;
      post_branching_ = post_branching;
      fair_loop_approx_ = fair_loop_approx;
      ltl_string_ = ltl_string;
      display_ = dotdump;
      scc_optim_ = scc_optim;
      scc_optim_full_ = scc_optim_full;
      print_formula_tgba_ = print_formula_tgba;
    }

    /// \brief Check if the Petri net \a n can produce at least one infinite
    /// sequence accepted by the formula \a f.
    ///
    /// If this is the case and \a ce_expected is true, such a sequence is displayed.
    /// In other case, a message indicating if the test is satisfied or not is
    /// printed. The four last parameters specify options for the translation
    /// of the formula \a f in a TGBA as indicated for the function
    /// spot::ltl_to_tgba_fm.
    /// \dontinclude modelcheck.cpp
    /// \skipline void model_check
    /// \until } //
    void model_check(sog_product_type sogtype);

  };

} // namespace

#endif
