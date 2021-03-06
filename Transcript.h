#ifndef TRANSCRIPT_H_
#define TRANSCRIPT_H_

#include<cstdio>
#include<cstdlib>
#include<cassert>
#include<string>
#include<vector>
#include<fstream>

#include "utils.h"

struct Interval {
	int start, end;

	Interval(int start, int end) {
		this->start = start;
		this->end = end;
	}
};

class Transcript {
public:
	Transcript() {
		length = 0;
		structure.clear();
		strand = 0;
		seqname = gene_id = transcript_id = "";
		left = "";
	}

	Transcript(const std::string& transcript_id, const std::string& gene_id, const std::string& seqname,
			const char& strand, const std::vector<Interval>& structure, const std::string& left) {
		this->structure = structure;
		this->strand = strand;
		this->seqname = seqname;
		this->gene_id = gene_id;
		this->transcript_id = transcript_id;

		//eliminate prefix spaces in string variable "left"
		int pos = 0;
		int len = left.length();
		while (pos < len && left[pos] == ' ') ++pos;
		this->left = left.substr(pos);

		length = 0;
		int s = structure.size();
		for (int i = 0; i < s; i++) length += structure[i].end + 1 - structure[i].start;
	}

	bool operator< (const Transcript& o) const {
	  return gene_id < o.gene_id || (gene_id == o.gene_id && transcript_id < o.transcript_id);
	}

	const std::string& getTranscriptID() const { return transcript_id; }

	const std::string& getGeneID() const { return gene_id; }

	const std::string& getSeqName() const { return seqname; }

	char getStrand() const { return strand; }

	const std::string& getLeft() const { return left; }

	int getLength() const { return length; }

	const std::vector<Interval>& getStructure() const { return structure; }

	void extractSeq (const std::string&, std::string&) const;

	void read(std::ifstream&);
	void write(std::ofstream&);

private:
	int length; // transcript length
	std::vector<Interval> structure; // transcript structure , coordinate starts from 1
	char strand;
	std::string seqname, gene_id, transcript_id; // follow GTF definition
	std::string left;
};

//gseq : genomic sequence
void Transcript::extractSeq(const std::string& gseq, std::string& seq) const {
	seq = "";
	int s = structure.size();
	size_t glen = gseq.length();

	if (structure[0].start < 1 || (size_t)structure[s - 1].end > glen) {
		fprintf(stderr, "Transcript %s is out of chromosome %s's boundary!\n", transcript_id.c_str(), seqname.c_str());
		exit(-1);
	}

	switch(strand) {
	case '+':
		for (int i = 0; i < s; i++) {
			seq += gseq.substr(structure[i].start - 1, structure[i].end - structure[i].start + 1); // gseq starts from 0!
		}
		break;
	case '-':
		for (int i = s - 1; i >= 0; i--) {
			for (int j = structure[i].end; j >= structure[i].start; j--) {
				seq += getOpp(gseq[j - 1]);
			}
		}
		break;
	default: assert(false);
	}

	assert(seq.length() > 0);
}

void Transcript::read(std::ifstream& fin) {
	int s;
	std::string tmp;

	getline(fin, transcript_id);
	getline(fin, gene_id);
	getline(fin, seqname);
	fin>>tmp>>length;
	assert(tmp.length() == 1 && (tmp[0] == '+' || tmp[0] == '-'));
	strand = tmp[0];
	structure.clear();
	fin>>s;
	for (int i = 0; i < s; i++) {
		int start, end;
		fin>>start>>end;
		structure.push_back(Interval(start, end));
	}
	getline(fin, tmp); //get the end of this line
	getline(fin, left);
}

void Transcript::write(std::ofstream& fout) {
	int s = structure.size();

	fout<<transcript_id<<std::endl;
	fout<<gene_id<<std::endl;
	fout<<seqname<<std::endl;
	fout<<strand<<" "<<length<<std::endl;
	fout<<s;
	for (int i = 0; i < s; i++) fout<<" "<<structure[i].start<<" "<<structure[i].end;
	fout<<std::endl;
	fout<<left<<std::endl;
}

#endif /* TRANSCRIPT_H_ */
