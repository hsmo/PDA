/***************************************************************************
 *   Copyright (C) 2009 by BUI Quang Minh   *
 *   minh.bui@univie.ac.at   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdarg.h>
#include "phylotree.h"
#include "superalignment.h"
#include "phylosupertree.h"

SuperAlignment::SuperAlignment()
 : Alignment() {}

SuperAlignment::SuperAlignment(PhyloSuperTree *super_tree)
 : Alignment()
{

	int site, seq, nsite = super_tree->size();
	PhyloSuperTree::iterator it;
	map<string,int> name_map;
	for (site = 0, it = super_tree->begin(); it != super_tree->end(); it++, site++) {
		partitions.push_back((*it)->aln);
		int nseq = (*it)->aln->getNSeq();
		//cout << "nseq  = " << nseq << endl;
		for (seq = 0; seq < nseq; seq++) {
			int id = getSeqID((*it)->aln->getSeqName(seq));
			if (id < 0) {
				seq_names.push_back((*it)->aln->getSeqName(seq));
				id = seq_names.size()-1;
				IntVector vec(nsite, -1);
				vec[site] = seq;
				taxa_index.push_back(vec);
			} else
				taxa_index[id][site] = seq;
		}
	}
	num_states = 2; // binary type because the super alignment presents the presence/absence of taxa in the partitions
	site_pattern.resize(nsite, -1);
	clear();
	pattern_index.clear();
	VerboseMode save_mode = verbose_mode; 
	verbose_mode = min(verbose_mode, VB_MIN); // to avoid printing gappy sites in addPattern
	int nseq = getNSeq();
	for (site = 0; site < nsite; site++) {
 		Pattern pat;
 		pat.append(nseq, 0);
		for (seq = 0; seq < nseq; seq++)
			pat[seq] = (taxa_index[seq][site] >= 0)? 1 : 0;
		addPattern(pat, site);
	}
	verbose_mode = save_mode;
	countConstSite();
}

void SuperAlignment::linkSubAlignment(int part) {
	assert(taxa_index.size() == getNSeq());
	int nseq = getNSeq(), seq;
	for (seq = 0; seq < nseq; seq++) {
		int id = partitions[part]->getSeqID(getSeqName(seq));
		if (id < 0)
			taxa_index[seq][part] = -1;
		else
			taxa_index[seq][part] = id;
	}
}

/*
void SuperAlignment::checkGappySeq() {
	int nseq = getNSeq(), part = 0, i;
	IntVector gap_only_seq;
	gap_only_seq.resize(nseq, 1);
	//cout << "Checking gaps..." << endl;
	for (vector<Alignment*>::iterator it = partitions.begin(); it != partitions.end(); it++, part++) {
		IntVector keep_seqs;
		for (i = 0; i < nseq; i++)
			if (taxa_index[i][part] >= 0)
			if (!(*it)->isGapOnlySeq(taxa_index[i][part])) {
				keep_seqs.push_back(taxa_index[i][part]);
				gap_only_seq[i] = 0;
			}
		if (keep_seqs.size() < (*it)->getNSeq()) {
			cout << "Discard " << (*it)->getNSeq() - keep_seqs.size() 
				 << " sequences from partition number " << part+1 << endl;
			Alignment *aln = new Alignment;
			aln->extractSubAlignment((*it), keep_seqs, 0);
			delete (*it);
			(*it) = aln;
			linkSubAlignment(part);
		}
		cout << __func__ << " num_states = " << (*it)->num_states << endl;
	}
	int wrong_seq = 0;
	for (i = 0; i < nseq; i++)
		if (gap_only_seq[i]) {
			cout << "ERROR: Sequence " << getSeqName(i) << " contains only gaps or missing data" << endl;
			wrong_seq++;
		}
	if (wrong_seq) {
		outError("Some sequences (see above) are problematic, please check your alignment again");
		}
}
*/
void SuperAlignment::getSitePatternIndex(IntVector &pattern_index) {
	for (vector<Alignment*>::iterator it = partitions.begin(); it != partitions.end(); it++) {
		int offset = pattern_index.size();
		pattern_index.insert(pattern_index.end(), (*it)->site_pattern.begin(), (*it)->site_pattern.end());
		for (int i = offset; i < pattern_index.size(); i++)
			pattern_index[i] += offset;
	}
}

void SuperAlignment::getPatternFreq(IntVector &pattern_freq) {
	if (!isSuperAlignment()) outError("Internal error: ", __func__);
	int offset = 0;
	if (!pattern_freq.empty()) pattern_freq.resize(0);
	for (vector<Alignment*>::iterator it = partitions.begin(); it != partitions.end(); it++) {
		IntVector freq;
		(*it)->getPatternFreq(freq);
		pattern_freq.insert(pattern_freq.end(), freq.begin(), freq.end());
		offset += freq.size();
	}
}

void SuperAlignment::createBootstrapAlignment(Alignment *aln, IntVector* pattern_freq, const char *spec) {
	if (!aln->isSuperAlignment()) outError("Internal error: ", __func__);
	if (pattern_freq) outError("Unsupported yet.", __func__);
	if (spec) outError("Unsupported yet.", __func__);
	Alignment::copyAlignment(aln);
	SuperAlignment *super_aln = (SuperAlignment*) aln;
	if (!partitions.empty()) outError("Internal error: ", __func__);
	for (vector<Alignment*>::iterator it = super_aln->partitions.begin(); it != super_aln->partitions.end(); it++) {
		Alignment *boot_aln = new Alignment;
		boot_aln->createBootstrapAlignment(*it);
		partitions.push_back(boot_aln);
	}
	taxa_index = super_aln->taxa_index;
}

void SuperAlignment::createBootstrapAlignment(IntVector &pattern_freq, const char *spec) {
	if (!isSuperAlignment()) outError("Internal error: ", __func__);
	int nptn = 0;
	for (vector<Alignment*>::iterator it = partitions.begin(); it != partitions.end(); it++) {
		nptn += (*it)->getNPattern();
	}
	pattern_freq.resize(0);
	int *internal_freq = new int[nptn];
	createBootstrapAlignment(internal_freq, spec);
	pattern_freq.insert(pattern_freq.end(), internal_freq, internal_freq + nptn);
	delete [] internal_freq;

/*	if (spec && strncmp(spec, "GENE", 4) != 0) outError("Unsupported yet.", __func__);

	int offset = 0;
	if (!pattern_freq.empty()) pattern_freq.resize(0);

	if (spec && strncmp(spec, "GENE", 4) == 0) {
		// resampling whole genes
		int nptn = 0;
		IntVector part_pos;
		for (vector<Alignment*>::iterator it = partitions.begin(); it != partitions.end(); it++) {
			part_pos.push_back(nptn);
			nptn += (*it)->getNPattern();
		}
		pattern_freq.resize(nptn, 0);
		for (int i = 0; i < partitions.size(); i++) {
			int part = random_int(partitions.size());
			for (int j = 0; j < partitions[part]->getNPattern(); j++)
				pattern_freq[j + part_pos[part]] += partitions[part]->at(j).frequency;
		}
	} else {
		// resampling sites within genes
		for (vector<Alignment*>::iterator it = partitions.begin(); it != partitions.end(); it++) {
			IntVector freq;
			(*it)->createBootstrapAlignment(freq);
			pattern_freq.insert(pattern_freq.end(), freq.begin(), freq.end());
			offset += freq.size();
		}
	}*/
}


void SuperAlignment::createBootstrapAlignment(int *pattern_freq, const char *spec) {
	if (!isSuperAlignment()) outError("Internal error: ", __func__);
	if (spec && strncmp(spec, "GENE", 4) != 0) outError("Unsupported yet. ", __func__);

	if (spec && strncmp(spec, "GENE", 4) == 0) {
		// resampling whole genes
		int nptn = 0;
		IntVector part_pos;
		for (vector<Alignment*>::iterator it = partitions.begin(); it != partitions.end(); it++) {
			part_pos.push_back(nptn);
			nptn += (*it)->getNPattern();
		}
		memset(pattern_freq, 0, nptn * sizeof(int));
		for (int i = 0; i < partitions.size(); i++) {
			int part = random_int(partitions.size());
			Alignment *aln = partitions[part];
			if (strncmp(spec,"GENESITE",8) == 0) {
				// then resampling sites in resampled gene
				for (int j = 0; j < aln->getNSite(); j++) {
					int ptn_id = aln->getPatternID(random_int(aln->getNPattern()));
					pattern_freq[ptn_id + part_pos[part]]++;
				}

			} else {
				for (int j = 0; j < aln->getNPattern(); j++)
					pattern_freq[j + part_pos[part]] += aln->at(j).frequency;
			}
		}
	} else {
		// resampling sites within genes
		int offset = 0;
		for (vector<Alignment*>::iterator it = partitions.begin(); it != partitions.end(); it++) {
			(*it)->createBootstrapAlignment(pattern_freq + offset);
			offset += (*it)->getNPattern();
		}
	}
}

/**
 * shuffle alignment by randomizing the order of sites
 */
void SuperAlignment::shuffleAlignment() {
	if (!isSuperAlignment()) outError("Internal error: ", __func__);
	for (vector<Alignment*>::iterator it = partitions.begin(); it != partitions.end(); it++) {
		(*it)->shuffleAlignment();
	}
}


double SuperAlignment::computeObsDist(int seq1, int seq2) {
	int site;
	int diff_pos = 0, total_pos = 0;
	for (site = 0; site < getNSite(); site++) {
		int id1 = taxa_index[seq1][site];
		int id2 = taxa_index[seq2][site];
		if (id1 < 0 || id2 < 0) continue;
		int num_states = partitions[site]->num_states;
		for (Alignment::iterator it = partitions[site]->begin(); it != partitions[site]->end(); it++) 
			if  ((*it)[id1] < num_states && (*it)[id2] < num_states) {
				total_pos += (*it).frequency;
				if ((*it)[id1] != (*it)[id2] )
					diff_pos += (*it).frequency;
			}
	}
	if (!total_pos) 
		return MAX_GENETIC_DIST; // return +INF if no overlap between two sequences
	return ((double)diff_pos) / total_pos;
}


double SuperAlignment::computeDist(int seq1, int seq2) {
	if (partitions.empty()) return 0.0;
	double obs_dist = computeObsDist(seq1, seq2);
    int num_states = partitions[0]->num_states;
    double z = (double)num_states / (num_states-1);
    double x = 1.0 - (z * obs_dist);

    if (x <= 0) {
        /*		string str = "Too long distance between two sequences ";
        		str += getSeqName(seq1);
        		str += " and ";
        		str += getSeqName(seq2);
        		outWarning(str);*/
        return MAX_GENETIC_DIST;
    }

    return -log(x) / z;
    //return computeObsDist(seq1, seq2);
	//  AVERAGE DISTANCE

	double dist = 0;
	int part = 0, num = 0;
	for (vector<Alignment*>::iterator it = partitions.begin(); it != partitions.end(); it++, part++) {
		int id1 = taxa_index[seq1][part];
		int id2 = taxa_index[seq2][part];
		if (id1 < 0 || id2 < 0) continue;
		dist += (*it)->computeDist(id1, id2);
	}
	if (num == 0) // two sequences are not overlapping at all!
		return MAX_GENETIC_DIST;
	return dist / num;
}

SuperAlignment::~SuperAlignment()
{
	for (vector<Alignment*>::reverse_iterator it = partitions.rbegin(); it != partitions.rend(); it++)
		delete (*it);
	partitions.clear();
}


void SuperAlignment::printCombinedAlignment(const char *file_name, bool append) {
	vector<Alignment*>::iterator pit;
	int final_length = 0;
	for (pit = partitions.begin(); pit != partitions.end(); pit++)
		final_length += (*pit)->getNSite();
	try {
		ofstream out;
		out.exceptions(ios::failbit | ios::badbit);

		if (append)
			out.open(file_name, ios_base::out | ios_base::app);
		else
			out.open(file_name);
		out << getNSeq() << " " << final_length << endl;
		StrVector::iterator it;
		int max_len = getMaxSeqNameLength();
		if (max_len < 10) max_len = 10;
		int seq_id = 0;
		for (it = seq_names.begin(); it != seq_names.end(); it++, seq_id++) {
			out.width(max_len);
			out << left << (*it) << " ";
			int part = 0;
			for (pit = partitions.begin(); pit != partitions.end(); pit++, part++) {
				int part_seq_id = taxa_index[seq_id][part];
				int nsite = (*pit)->getNSite();
				if (part_seq_id >= 0) {
					for (int i = 0; i < nsite; i++)
						out << (*pit)->convertStateBackStr((*pit)->getPattern(i) [part_seq_id]);
				} else {
					string str(nsite, '?');
					out << str;
				}
			}
			out << endl;
		}
		out.close();
		cout << "Concatenated alignment was printed to " << file_name << endl;
	} catch (ios::failure) {
		outError(ERR_WRITE_OUTPUT, file_name);
	}	
}

void SuperAlignment::printSubAlignments(Params &params, vector<PartitionInfo> &part_info) {
	vector<Alignment*>::iterator pit;
	string filename;
	int part;
	assert(part_info.size() == partitions.size());
	for (pit = partitions.begin(), part = 0; pit != partitions.end(); pit++, part++) {
		if (params.aln_output)
			filename = params.aln_output;
		else
			filename = params.out_prefix;
		filename += "." + part_info[part].name;
		 if (params.aln_output_format == ALN_PHYLIP)
			(*pit)->printPhylip(filename.c_str(), false, NULL, params.aln_nogaps, NULL);
		else if (params.aln_output_format == ALN_FASTA)
			(*pit)->printFasta(filename.c_str(), false, NULL, params.aln_nogaps, NULL);
	}
}

double SuperAlignment::computeUnconstrainedLogL() {
	double logl = 0.0;
	vector<Alignment*>::iterator pit;
	for (pit = partitions.begin(); pit != partitions.end(); pit++)
		logl += (*pit)->computeUnconstrainedLogL();
	return logl;
}

double SuperAlignment::computeMissingData() {
	double ret = 0.0;
	int len = 0;
	vector<Alignment*>::iterator pit;
	for (pit = partitions.begin(); pit != partitions.end(); pit++) {
		ret += (*pit)->getNSeq() * (*pit)->getNSite();
		len += (*pit)->getNSite();
	}
	ret /= getNSeq() * len;
	return 1.0 - ret;

}

Alignment *SuperAlignment::concatenateAlignments(IntVector &ids) {
	string union_taxa;
	int nsites = 0, nstates = 0, i;
	for (i = 0; i < ids.size(); i++) {
		int id = ids[i];
		if (id < 0 || id >= partitions.size())
			outError("Internal error ", __func__);
		if (nstates == 0) nstates = partitions[id]->num_states;
		if (nstates != partitions[id]->num_states)
			outError("Cannot concatenate sub-alignments of different type");

		string taxa_set = getPattern(id);
		nsites += partitions[id]->getNSite();
		if (i == 0) union_taxa = taxa_set; else {
			for (int j = 0; j < union_taxa.length(); j++)
				if (taxa_set[j] == 1) union_taxa[j] = 1;
		}
	}

	Alignment *aln = new Alignment;
	for (i = 0; i < union_taxa.length(); i++)
		if (union_taxa[i] == 1) {
			aln->seq_names.push_back(getSeqName(i));
		}
	aln->num_states = nstates;
	aln->site_pattern.resize(nsites, -1);
    aln->clear();
    aln->pattern_index.clear();

    int site = 0;
    for (i = 0; i < ids.size(); i++) {
    	int id = ids[i];
		string taxa_set = getPattern(id);
    	for (Alignment::iterator it = partitions[id]->begin(); it != partitions[id]->end(); it++, site++) {
    		Pattern pat;
    		int part_seq = 0;
    		for (int seq = 0; seq < union_taxa.size(); seq++)
    			if (union_taxa[seq] == 1) {
    				char ch = STATE_UNKNOWN;
    				if (taxa_set[seq] == 1) {
    					ch = (*it)[part_seq++];
    				}
    				pat.push_back(ch);
    			}
    		assert(part_seq == partitions[id]->getNSeq());
    		aln->addPattern(pat, site, (*it).frequency);
    	}
    }
    aln->countConstSite();

	return aln;
}
