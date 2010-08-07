
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

	// move to document
	void DRC();

private:
	QList<Part> m_parts;	// the actual list of parts
	int m_max_size;
	int m_layers;
	int m_annular_ring;
	CNetList * m_nlist;
	QMap<QString,Footprint*> * m_footprint_cache_map;


};
 
