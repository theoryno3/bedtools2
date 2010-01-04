/*****************************************************************************
  pairToBed.h

  (c) 2009 - Aaron Quinlan
  Hall Laboratory
  Department of Biochemistry and Molecular Genetics
  University of Virginia
  aaronquinlan@gmail.com

  Licenced under the GNU General Public License 2.0+ license.
******************************************************************************/
#ifndef INTERSECTBED_H
#define INTERSECTBED_H

#include "BamReader.h"
#include "BamWriter.h"
#include "BamAux.h"
using namespace BamTools;

#include "bedFile.h"
#include "bedFilePE.h"
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

//************************************************
// Class methods and elements
//************************************************
class BedIntersectPE {

public:

	// constructor 
	BedIntersectPE(string bedAFilePE, string bedBFile, float overlapFraction, 
		string searchType, bool forceStrand, bool bamInput, bool bamOutput);

	// destructor
	~BedIntersectPE(void);

	void FindOverlaps(BEDPE &, vector<BED> &a, vector<BED> &hits, string &type); 
	bool FindOneOrMoreOverlaps(BEDPE &, vector<BED> &a, vector<BED> &hits, string &type); 

	void FindSpanningOverlaps(BEDPE &a, vector<BED> &hits, string &type); 
	bool FindOneOrMoreSpanningOverlaps(BEDPE &a, vector<BED> &hits, string &type);

	void IntersectBedPE(istream &bedInput);
	void IntersectBamPE(string bamFile);
	
	void DetermineBedPEInput();

	
private:

	string bedAFilePE;
	string bedBFile;
	float overlapFraction;
	string searchType;
	bool forceStrand;
	bool bamInput;
	bool bamOutput;

	// instance of a paired-end bed file class.
	BedFilePE *bedA;

	// instance of a bed file class.
	BedFile *bedB;
	
	inline void ConvertBamToBedPE(const BamAlignment &bam, const RefVector &refs, BEDPE &a) {
		a.chrom1 = refs.at(bam.RefID).RefName;
		a.start1 = bam.Position;
		a.end1 = bam.Position + bam.Length;
		a.chrom2 = refs.at(bam.MateRefID).RefName;
		a.start2 = bam.MatePosition;
		a.end2 = bam.MatePosition + bam.Length;
		a.name = bam.Name;
		a.score = "1";
		a.strand1 = "+"; if (bam.IsReverseStrand()) a.strand1 = "-";
		a.strand2 = "+"; if (bam.IsMateReverseStrand()) a.strand2 = "-";
	};
};

#endif /* PEINTERSECTBED_H */
