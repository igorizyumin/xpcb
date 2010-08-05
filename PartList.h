
#pragma once

#include "Shape.h"
#include "smfontutil.h"
#include "UndoList.h"

#define MAX_REF_DES_SIZE 39

class Part;
class PartList;
class CNetList;
class Net;

#include "DesignRules.h"

// clearance types for GetPadDrawInfo()
enum {
	CLEAR_NORMAL = 0,
	CLEAR_THERMAL,
	CLEAR_NONE
};



// partlist_info is used to hold digest of CPartList 
// for editing in dialogs, or importing from netlist file
// notes:
//	package may be "" if no package assigned
//	shape may be NULL if no footprint assigned
//	may have package but no footprint, but not the reverse
typedef struct {
	Part * part;		// pointer to original part, or NULL if new part added
	CString ref_des;	// ref designator string
	int ref_size;		// size of ref text characters
	int ref_width;		// stroke width of ref text characters
	CString package;	// package (from original imported netlist, don't edit)
	CString value;		// value (from original imported netlist, don't edit)
	bool value_vis;		// visibility of value
	Footprint * shape;		// pointer to shape (may be edited)
	bool deleted;		// flag to indicate that part was deleted
	bool bShapeChanged;	// flag to indicate that the shape has changed
	bool bOffBoard;		// flag to indicate that position has not been set
	int x, y;			// position (may be edited)
	int angle, side;	// angle and side (may be edited)
} part_info;

typedef CArray<part_info> partlist_info;

// error codes
enum
{
	PL_NOERR = 0,
	PL_NO_DLIST,
	PL_NO_FOOTPRINT,
	PL_ERR
};

class PartList
{
public:
	enum {
		NOT_CONNECTED = 0,		// pin not attached to net
		ON_NET = 1,				// pin is attached to a net
		TRACE_CONNECT = 2,		// pin connects to trace on this layer
		AREA_CONNECT = 4		// pin connects to copper area on this layer
	};

	// TODO replace these with methods that return iterators for QList
	Part m_start, m_end;

public:
	enum { 
		UNDO_PART_DELETE=1, 
		UNDO_PART_MODIFY, 
		UNDO_PART_ADD };	// undo types

	PartList( SMFontUtil * fontutil );
	~PartList();


	// getter/setter functions
	int GetNumParts(){ return m_parts.size(); }
	void SetShapeCacheMap( QMap<QString, Footprint*> * shape_cache_map ) { m_footprint_cache_map = shape_cache_map; }
	void SetNetlist( CNetList * nlist ){ m_nlist = nlist; }
	void SetNumCopperLayers( int nlayers ){ m_layers = nlayers;}
	void SetPinAnnularRing( int ring ){ m_annular_ring = ring; }
	int GetPartBoundaries( CRect * part_r );
	int GetNumFootprintInstances( Footprint * shape );

	// container functions
	Part * Add();
	Part * Add( Footprint * shape, CString * ref_des, CString * package,
					int x, int y, int side, int angle, int visible, int glued ); 
	Part * AddFromString( CString * str );
	int Remove( Part * element );
	void RemoveAllParts();
	Part * GetPart( QString ref_des );


	void MarkAllParts( int mark );
	void HighlightAllPadsOnNet( Net * net );
	int GetPadDrawInfo( Part * part, int ipin, int layer,
							  bool bUse_TH_thermals, bool bUse_SMT_thermals,
							  int mask_clearance, int paste_mask_shrink,
							  int * type=0, int * x=0, int * y=0, int * w=0, int * l=0, int * r=0, int * hole=0,
							  int * angle=0, Net ** net=0,
							  int * connection_status=0, int * pad_connect_flag=0,
							  int * clearance_type=0 );




	void MoveOrigin( int x_off, int y_off );


	void FootprintChanged( Footprint * shape );
	void RefTextSizeChanged( Footprint * shape );

	int WriteParts( CStdioFile * file );
	int ReadParts( CStdioFile * file );
	int ExportPartListInfo( partlist_info * pl, Part * part );
	void ImportPartListInfo( partlist_info * pl, int flags, CDlgLog * log=NULL );


	void PurgeFootprintCache();

	// check functions
	int CheckPartlist( CString * logstr );
	bool CheckForProblemFootprints();
	void DRC( CDlgLog * log, int copper_layers, 
		int units, bool check_unrouted,
		CArray<PolyLine> * board_outline,
		DesignRules * dr, DRErrorList * DRElist );

private:
	QList<Part> m_parts;	// the actual list of parts
	int m_max_size;
	int m_layers;
	int m_annular_ring;
	CNetList * m_nlist;
	SMFontUtil * m_fontutil;	// class for Hershey font
	QMap<QString,Footprint*> * m_footprint_cache_map;


};
 
