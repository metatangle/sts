#include "node_slider_mcmc_move.h"
#include "composite_tree_likelihood.h"
#include "multiplier_proposal.h"
#include "online_util.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <utility>
#include <unordered_set>

#if defined(_OPENMP)
#include <omp.h>
#endif

using namespace bpp;

namespace sts { namespace online {

	NodeSliderMCMCMove::NodeSliderMCMCMove(std::vector<std::unique_ptr<CompositeTreeLikelihood>>& calculator, const std::vector<std::string>& parameters,
                                       const double lambda) :
    OnlineMCMCMove(calculator, parameters, lambda)
{}

NodeSliderMCMCMove::~NodeSliderMCMCMove()
{
    // Debug bits
    if(n_attempted > 0) {
        std::clog << "Node_slider_mcmc_move: " << n_accepted << '/' << n_attempted << ": " << acceptanceProbability() << std::endl;
    }
}

int NodeSliderMCMCMove::proposeMove(long, smc::particle<TreeParticle>& particle, smc::rng* rng)
{
    // Choose an edge at random
    TreeParticle* value = particle.GetValuePointer();
    return proposeMove(*value, rng);
}

int NodeSliderMCMCMove::proposeMove(TreeParticle& particle, smc::rng* rng)
{
	size_t index = 0;
#if defined(_OPENMP)
	index = omp_get_thread_num();
#endif
	
    TreeTemplate<bpp::Node>* tree = particle.tree.get();
    std::vector<bpp::Node*> nodes = onlineAvailableEdges(*tree);
    
    // restrict to nodes with a parent in the available set
    const std::unordered_set<Node*> node_set(nodes.begin(), nodes.end());
    auto father_unavailable = [&node_set](Node* node) {
        return node_set.find(node->getFather()) == node_set.end();
    };
    nodes.erase(std::remove_if(nodes.begin(), nodes.end(), father_unavailable), nodes.end());
    
    size_t idx = rng->UniformDiscrete(0, nodes.size() - 1);
    Node* n = nodes[idx];
    
    Node* father = n->getFather();
    
    const double orig_dist = n->getDistanceToFather() + father->getDistanceToFather();
    
    const double orig_n_dist = n->getDistanceToFather();
    const double orig_father_dist = father->getDistanceToFather();
    
    _calculator[index]->initialize(*particle.model,
                          *particle.rateDist,
                          *tree);
    double orig_ll = particle.logP;
    
    const Proposal p = positive_real_multiplier(orig_dist, 1e-6, 100.0, _lambda, rng);
    const double d = rng->UniformS() * p.value;
    
    n->setDistanceToFather(d);
    father->setDistanceToFather(p.value - d);
    
	double new_ll = _calculator[index]->operator()();
    particle.logP = new_ll;
    
    double mh_ratio = std::exp(new_ll + std::log(p.hastingsRatio) - orig_ll);
    if(mh_ratio >= 1.0 || rng->UniformS() < mh_ratio) {
        return 1;
    } else {
        // Rejected
        particle.logP = orig_ll;
        n->setDistanceToFather(orig_n_dist);
        father->setDistanceToFather(orig_father_dist);
        return 0;
    }
}
    
}}
