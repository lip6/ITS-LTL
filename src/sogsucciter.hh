#ifndef SOG_SUCC_ITERATOR_HH
#define SOG_SUCC_ITERATOR_HH

#include <string>
#include <map>

#include "tgba/bdddict.hh"
#include "tgba/succiter.hh"
#include "tgba/state.hh"

#include "bdd.h"
#include "apiterator.hh"

#include "sogIts.hh"
#include "sogstate.hh"


namespace sogits {

/// \brief Implementation of a \a spot::tgba_succ_iterator for a \a ::marking.
class sog_succ_iterator : public spot::tgba_succ_iterator {
  public:
  sog_succ_iterator(const sogIts & m, const sog_state& s);
  virtual ~sog_succ_iterator();

    void first();
    void step();
    void next();
    bool done() const;
    spot::state* current_state() const;
    bdd current_condition() const;
    int current_transition() const;
    bdd current_acceptance_conditions() const;
  
  // pretty print
  std::ostream & print (std::ostream &) const ;
private:

  const sogIts& model; ///< The petri net.
  const sog_state & from; ///< The source state.

  APIterator it;
  bool div_needs_visit;
  its::State succstates;
  sog_state * current_succ;
};

class sog_div_succ_iterator : public spot::tgba_succ_iterator {
  public:
    sog_div_succ_iterator(const spot::bdd_dict* d, const bdd& c);

    void first();
    void next();
    bool done() const;
    spot::state* current_state() const;
    bdd current_condition() const;
    int current_transition() const;
    bdd current_acceptance_conditions() const;
    std::string format_transition() const;

  private:
    sog_div_succ_iterator(const sog_div_succ_iterator& s);
    sog_div_succ_iterator& operator=(const sog_div_succ_iterator& s);

    const spot::bdd_dict* dict;
    bdd cond; ///< The condition which must label the unique successor.
    bool div_has_been_visited;
};


} // namespace sogits 


std::ostream & operator << (std::ostream & , const sogits::sog_succ_iterator &);


#endif
