#ifndef STS_ONLINE_ONLINE_MCMC_MOVE_H
#define STS_ONLINE_ONLINE_MCMC_MOVE_H

#include <smctc.hh>

#include <Bpp/Numeric/Parameter.h>

#include "composite_tree_likelihood.h"

namespace sts { namespace online {

// Forwards
class TreeParticle;

class OnlineMCMCMove
{
public:
    OnlineMCMCMove(std::vector<std::unique_ptr<CompositeTreeLikelihood>>& calculators, const std::vector<std::string>& parameters={}, double lambda=3);
    virtual ~OnlineMCMCMove() {};

    double acceptanceProbability() const;

    int operator()(long, smc::particle<TreeParticle>&, smc::rng*);
    
    int operator()(TreeParticle&, smc::rng*);
    
protected:
    virtual int proposeMove(long time, smc::particle<TreeParticle>& particle, smc::rng* rng) = 0;
    
    virtual int proposeMove(TreeParticle& particle, smc::rng* rng) = 0;

    virtual double tune();
    
    /// Number of times the move was attempted
    unsigned int n_attempted;
    /// Number of times the move was accepted
    unsigned int n_accepted;
    
    double _lambda;
    
    double _target;
    double _min;
    double _max;
	std::vector<std::string> _parameters;
	
	std::vector<std::unique_ptr<CompositeTreeLikelihood>>& _calculator;
};

}}

#endif
