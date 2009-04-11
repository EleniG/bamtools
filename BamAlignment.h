// BamAlignment.h

// Derek Barnett
// Marth Lab, Boston College
// Last modified: 20 March 2009

#ifndef BAMALIGNMENT_H
#define BAMALIGNMENT_H

#include <string.h>
#include <stdlib.h>

#ifdef WIN32
typedef char                 int8_t;
typedef unsigned char       uint8_t;
typedef short               int16_t;
typedef unsigned short     uint16_t;
typedef int                 int32_t;
typedef unsigned int       uint32_t;
typedef long long           int64_t;
typedef unsigned long long uint64_t;
#else
#include <stdint.h>
#endif

// C++ includes
#include <string>
using std::string;

#include <vector>
using std::vector;

struct CigarOp {
	uint32_t Length;
	char     Type;
};

struct RefData {
	string       RefName;
	unsigned int RefLength;
	bool         RefHasAlignments;

	// constructor
	RefData(void)
		: RefLength(0)
		, RefHasAlignments(false)
	{ }
};

typedef vector<RefData> RefVector;

struct BamAlignment {

	// queries against alignment flag - see below for further detail
public:
	bool IsPaired(void) const            { return ( (AlignmentFlag & PAIRED)        != 0 ); }
	bool IsProperPair(void) const        { return ( (AlignmentFlag & PROPER_PAIR)   != 0 ); }
	bool IsMapped(void) const            { return ( (AlignmentFlag & UNMAPPED)      == 0 ); }
	bool IsMateMapped(void) const        { return ( (AlignmentFlag & MATE_UNMAPPED) == 0 ); }
	bool IsReverseStrand(void) const     { return ( (AlignmentFlag & REVERSE)       != 0 ); }
	bool IsMateReverseStrand(void) const { return ( (AlignmentFlag & MATE_REVERSE)  != 0 ); }
	bool IsFirstMate(void) const         { return ( (AlignmentFlag & READ_1)        != 0 ); }
	bool IsSecondMate(void) const        { return ( (AlignmentFlag & READ_2)        != 0 ); }
	bool IsPrimaryAlignment(void) const  { return ( (AlignmentFlag & SECONDARY)     == 0 ); }
	bool IsFailedQC(void) const          { return ( (AlignmentFlag & QC_FAILED)     != 0 ); }
	bool IsDuplicate(void) const         { return ( (AlignmentFlag & DUPLICATE)     != 0 ); }

	// returns true and assigns the read group if present in the tag data
	bool GetReadGroup(string& readGroup) const {

		if(TagData.empty()) return false;

		// localize the tag data
		char* pTagData = (char*)TagData.data();
		const unsigned int tagDataLen = TagData.size();
		unsigned int numBytesParsed = 0;

		bool foundReadGroupTag = false;
		while(numBytesParsed < tagDataLen) {

			const char* pTagType = pTagData;
			const char* pTagStorageType = pTagData + 2;
			pTagData       += 3;
			numBytesParsed += 3;

			// check the current tag
			if(strncmp(pTagType, "RG", 2) == 0) {
				foundReadGroupTag = true;
				break;
			}

			// get the storage class and find the next tag
			SkipToNextTag(*pTagStorageType, pTagData, numBytesParsed);
		}

		// return if the read group tag was not present
		if(!foundReadGroupTag) return false;

		// assign the read group
		const unsigned int readGroupLen = strlen(pTagData);
		readGroup.resize(readGroupLen);
		memcpy((char*)readGroup.data(), pTagData, readGroupLen);
		return true;
	}

	// skips to the next tag
	static void SkipToNextTag(const char storageType, char* &pTagData, unsigned int& numBytesParsed) {
		switch(storageType) {
			case 'A':
			case 'c':
			case 'C':
				numBytesParsed++;
				pTagData++;
				break;
			case 's':
			case 'S':
			case 'f':
				numBytesParsed += 2;
				pTagData       += 2;
				break;
			case 'i':
			case 'I':
				numBytesParsed += 4;
				pTagData       += 4;
				break;
			case 'Z':
			case 'H':
				while(*pTagData) {
					numBytesParsed++;
					pTagData++;
				}
				break;
			default:
				printf("ERROR: Unknown tag storage class encountered: [%c]\n", *pTagData);
				exit(1);
		}
	}

	// data members
public:
	string       Name;           // read name
	unsigned int Length;         // query length
	string       QueryBases;     // original sequence ( produced from machine )
	string       AlignedBases;   // aligned sequence ( with indels ) 
	string       Qualities;      // FASTQ qualities ( still in ASCII characters )
	string       TagData;        // contains the tag data (accessor methods will pull the requested information out)
	unsigned int RefID;          // ID for reference sequence
	unsigned int Position;       // position on reference sequence where alignment starts
	unsigned int Bin;            // bin in BAM file where this alignment resides
	unsigned int MapQuality;     // mapping quality 
	unsigned int AlignmentFlag;  // see above for available queries
	vector<CigarOp> CigarData;   // vector of CIGAR operations (length & type) )
	unsigned int MateRefID;      // ID for reference sequence that mate was aligned to
	unsigned int MatePosition;   // position that mate was aligned to
	unsigned int InsertSize;     // mate pair insert size

	// alignment flag query constants
private:
	enum { PAIRED        = 1,		// Alignment comes from paired-end data
		PROPER_PAIR   = 2,		// Alignment passed paired-end resolution
		UNMAPPED      = 4,		// Read is unmapped
		MATE_UNMAPPED = 8,		// Mate is unmapped
		REVERSE       = 16,		// Read is on reverse strand
		MATE_REVERSE  = 32,		// Mate is on reverse strand
		READ_1        = 64, 		// This alignment is mate 1 of pair
		READ_2        = 128,		// This alignment is mate 2 of pair
		SECONDARY     = 256,		// This alignment is not the primary (best) alignment for read
		QC_FAILED     = 512,		// Read did not pass prior quality control steps
		DUPLICATE     = 1024		// Read is PCR duplicate
	};
};

// commonly used vector in this library
typedef vector< BamAlignment > BamAlignmentVector;

#endif /* BAMALIGNMENT_H */
