/*
 * phylosupertreeplen.h
 *
 *  Created on: Aug 5, 2013
 *      Author: olga
 */

#ifndef PHYLOSUPERTREEPLEN_H_
#define PHYLOSUPERTREEPLEN_H_

#include "phylosupertree.h"
#include "partitionmodel.h"
#include "superalignmentpairwise.h"

/**
Edge lengths in subtrees are proportional to edge lengths in a supertree.

	@author Olga Chernomor <olga.chernomor@univie.ac.at>
*/

class PhyloSuperTreePlen;

// Auxiliary classes ====================================================================================
// ======================================================================================================
class SuperAlignmentPairwisePlen : public SuperAlignmentPairwise {

	public:

		/**
		    constructor
		 */

	    SuperAlignmentPairwisePlen();

		/**
			construct the pairwise alignment from two sequences of a multiple alignment
			@param aln input multiple alignment
			@param seq_id1 ID of the first sequence
			@param seq_id2 ID of the second sequence
		*/
		SuperAlignmentPairwisePlen(PhyloSuperTreePlen *atree, int seq1, int seq2);

	    ~SuperAlignmentPairwisePlen();

		/**
			compute the likelihood for a distance between two sequences. Used for the ML optimization of the distance.
			@param value x-value of the function
			@return log-likelihood
		*/
		virtual double computeFunction(double value);

		/**
			This function calculate f(value), first derivative f'(value) and 2nd derivative f''(value).
			used by Newton raphson method to minimize the function.
			@param value x-value of the function
			@param df (OUT) first derivative
			@param ddf (OUT) second derivative
			@return f(value) of function f you want to minimize
		*/
		virtual double computeFuncDerv(double value, double &df, double &ddf);

		/**
			partition information
		*/
		vector<PartitionInfo>* part_info;

};
// ======================================================================================================
class PartitionModelPlen : public PartitionModel
{
public:
    PartitionModelPlen();
	/**
		constructor
		create partition model with possible rate heterogeneity. Create proper class objects
		for two variables: model and site_rate. It takes the following field of params into account:
			model_name, num_rate_cats, freq_type, store_trans_matrix
		@param params program parameters
		@param tree associated phylogenetic super-tree
	*/
	PartitionModelPlen(Params &params, PhyloSuperTreePlen *tree);

    ~PartitionModelPlen();

    /**
     * @return #parameters of the model + # branches
     */
    virtual int getNParameters();
    virtual int getNDim();

	/**
		optimize model parameters and tree branch lengths
		@param fixed_len TRUE to fix branch lengths, default is false
		@return the best likelihood
	*/
	virtual double optimizeParameters(bool fixed_len = false, bool write_info = true, double epsilon = 0.001);

	double optimizeGeneRate(double tol);

	virtual double targetFunk(double x[]);
	virtual void getVariables(double *variables);
	virtual void setVariables(double *variables);

};

// ======================================================================================================
// ======================================================================================================

class PhyloSuperTreePlen : public PhyloSuperTree {

public:
	/**
		constructors
	*/
	PhyloSuperTreePlen();
	PhyloSuperTreePlen(Params &params);
	PhyloSuperTreePlen(SuperAlignment *alignment, PhyloSuperTree *super_tree);

	~PhyloSuperTreePlen();

    /**
            compute the distance between 2 sequences.
            @param seq1 index of sequence 1
            @param seq2 index of sequence 2
            @param initial_dist initial distance
            @return distance between seq1 and seq2
     */

    virtual double computeDist(int seq1, int seq2, double initial_dist, double &var);

	/**
		create sub-trees T|Y_1,...,T|Y_k of the current super-tree T
		and map F={f_1,...,f_k} the edges of supertree T to edges of subtrees T|Y_i
	*/
	virtual void mapTrees();


	virtual double computeFuncDerv(double value, double &df, double &ddf);
	virtual double computeFunction(double value);

    /**
            optimize all branch lengths of all subtrees, then compute branch lengths
            of supertree as weighted average over all subtrees
            @param iterations number of iterations to loop through all branches
            @return the likelihood of the tree
     */
    virtual double optimizeAllBranches(int my_iterations = 100, double tolerance = TOL_LIKELIHOOD);

    /**
            optimize one branch length by ML by optimizing all mapped branches of subtrees
            @param node1 1st end node of the branch
            @param node2 2nd end node of the branch
            @param clearLH true to clear the partial likelihood, otherwise false
            @return likelihood score
     */
    virtual double optimizeOneBranch(PhyloNode *node1, PhyloNode *node2, bool clearLH = true);

    /**
            search the best swap for a branch
            @return NNIMove The best Move/Swap
            @param cur_score the current score of the tree before the swaps
            @param node1 1 of the 2 nodes on the branch
            @param node2 1 of the 2 nodes on the branch
     */
    virtual NNIMove getBestNNIForBran(PhyloNode *node1, PhyloNode *node2, NNIMove *nniMoves = NULL, bool approx_nni = false, bool useLS = false, double lh_contribution = -1.0);

    /**
            Do an NNI on the supertree and synchronize all subtrees respectively
            @param move the single NNI
     */
    virtual void doNNI(NNIMove &move, bool clearLH = true);
    /**
            apply nni2apply NNIs from the non-conflicting NNI list
            @param nni2apply number of NNIs to apply from the list
            @param changeBran whether or not the computed branch lengths should be applied
     */
    virtual void applyNNIs(int nni2apply, bool changeBran = true);


    /**
            This is for ML. try to swap the tree with nearest neigbor interchange at the branch connecting node1-node2.
            If a swap shows better score, return the swapped tree and the score.
            @param cur_score current likelihood score
            @param node1 1st end node of the branch
            @param node2 2nd end node of the branch
            @param nni_param (OUT) if not NULL: swapping information returned
            @return the likelihood of the tree
     */
    virtual double swapNNIBranch(double cur_score, PhyloNode *node1, PhyloNode *node2, SwapNNIParam *nni_param = NULL);

    /**
     *	used in swapNNIBranch to update link_neighbors of other SuperNeighbors that point to the same branch on SubTree as (node,dad)
     *	@param saved_link_dad_nei   pointer to link_neighbor dad_nei
     */
    void linkCheck(int part, Node* node, Node* dad, PhyloNeighbor* saved_link_dad_nei);
    void linkCheckRe(int part, Node* node, Node* dad, PhyloNeighbor* saved_link_dad_nei,PhyloNeighbor* saved_link_node_nei);

	/**
		compute the weighted average of branch lengths over partitions
	*/
	virtual void computeBranchLengths();

	bool checkBranchLen();
	void mapBranchLen();
	virtual void printMapInfo();

	virtual void restoreAllBranLen(PhyloNode *node, PhyloNode *dad);

	/**
	 * initialize partition information for super tree
	*/
	virtual void initPartitionInfo();

	void printNNIcasesNUM();

    /**
     * 		indicates whether partition rates are fixed or not
     */

    bool fixed_rates;

    /*
     * 1 - # of is_nni on subtree
     * 2 - # of relink branch to an empty one
     * 3 - # of empty to empty
     * 4 - # of relink branch to a  new one (50% saving on these cases compared to the previous implementation)
     * 5 - # of relink branch to an old one + relink empty to some branch (100% saving on these cases)
     */
    int allNNIcases_computed[5];
};


#endif /* PHYLOSUPERTREEPLEN_H_ */
