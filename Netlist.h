#ifndef NETLIST_H
#define NETLIST_H

#include <QString>
#include <QList>
#include "PolyLine.h"

// Forward declarations
class Net;
class Pin;
class Connection;
class Segment;
class Vertex;
class Part;
class Area;

// Basic types


// Net: describes a net
class Net
{
public:
	id id;				// net id
	QString name;		// net name
	int nconnects;		// number of connections
	QList<Connection> connect; // array of connections (size = max_pins-1)
	int npins;			// number of pins
	QList<Pin> pin;	// array of pins
	int nareas;			// number of copper areas
	QList<Area,Area> area;	// array of copper areas
	int def_w;			// default trace width
	int def_via_w;		// default via width
	int def_via_hole_w;	// default via hole width
	bool visible;		// FALSE to hide ratlines and make unselectable
	int utility;		// used to keep track of which nets have been optimized
	int utility2;		// used to keep track of which nets have been optimized
};

// cconnect: describes a connection between two pins or a stub trace with no end pin
class Connection
{
public:
	enum {
		NO_END = -1		// used for end_pin if stub trace
	};
	Connection()
	{	// constructor
		locked = 0;
		nsegs = 0;
		seg.SetSize( 0 );
		vtx.SetSize( 0 );
		utility = 0;
	};
	int start_pin, end_pin;		// indexes into net.pin array
	int nsegs;					// # elements in seg array
	int locked;					// 1 if locked (will not be optimized away)
	QList<Segment> seg;			// array of segments
	QList<Vertex> vtx;		// array of vertices, size = nsegs + 1
	int utility;				// used for various temporary ops
	// these params used only by DRC
	int min_x, max_x;			// bounding rect
	int min_y, max_y;
	bool vias_present;
	int seg_layers;
};


// Vertex: describes a vertex between segments
class Vertex
{
public:
	Vertex()
	{
		// constructor
		m_dlist = 0;	// this must set with Initialize()
		x = 0; y = 0;
		pad_layer = 0;	// only for first or last
		force_via_flag = 0;		// only used for end of stub trace
		via_w = 0;
		via_hole_w = 0;
		tee_ID = 0;
		utility = 0;
		utility2 = 0;
	}
	int x, y;					// coords
	int pad_layer;				// layer of pad if this is first or last vertex, otherwise 0
	int force_via_flag;			// force a via even if no layer change
	int via_w, via_hole_w;		// via width and hole width (via_w==0 means no via)
	int tee_ID;					// used to flag a t-connection point
	int utility, utility2;		// used for various functions
};

// Segment: describes a segment of a connection
class Segment
{
public:
	Segment()
	{
		// constructor
		layer = 0;
		width = 0;
		selected = 0;
		utility = 0;
	}
	int layer;				// copper layer
	int width;				// width
	int selected;			// 1 if selected for editing
	int utility;
};

// carea: describes a copper area
class Area
{
public:
	Area();
	~Area();						// destructor
	CPolyLine * poly;	// outline
	int npins;			// number of thru-hole pins within area on same net
	QList<int> pin;	// array of thru-hole pins
	int nvias;			// number of via connections to area
	QList<int> vcon;	// connections
	QList<int> vtx;	// vertices
	int utility, utility2;
};

// cpin: describes a pin in a net
class Pin
{
public:
	Pin(){ part = NULL; }
	QString ref_des;	// reference designator such as 'U1'
	QString pin_name;	// pin name such as "1" or "A23"
	Part * part;		// pointer to part containing the pin
	int utility;
};

// CNetlist
class CNetList
{
public:
	enum{ MAX_ITERATORS=10 };
	enum {
		VIA_NO_CONNECT = 0,
		VIA_TRACE = 1,
		VIA_AREA = 2
	};
	enum {						// used for UNDO records
		UNDO_CONNECT_MODIFY=1,	// undo modify connection
		UNDO_AREA_CLEAR_ALL,	// flag to remove all areas
		UNDO_AREA_ADD,			// undo add area (i.e. delete area)
		UNDO_AREA_MODIFY,		// undo modify area
		UNDO_AREA_DELETE,		// undo delete area (i.e. add area)
		UNDO_NET_ADD,			// undo add net (i.e delete net)
		UNDO_NET_MODIFY,		// undo modify net
		UNDO_NET_OPTIMIZE		// flag to optimize net on undo
	};
	CMapStringToPtr m_map;	// map net names to pointers
	CNetList( CDisplayList * dlist, CPartList * plist );
	~CNetList();
	void SetNumCopperLayers( int layers ){ m_layers = layers;};
	void SetWidths( int w, int via_w, int via_hole_w );
	void SetViaAnnularRing( int ring ){ m_annular_ring = ring; };
	void SetSMTconnect( BOOL bSMTconnect ){ m_bSMT_connect = bSMTconnect; };

	// functions for nets and pins
	void MarkAllNets( int utility );
	void MoveOrigin( int x_off, int y_off );
	cnet * GetNetPtrByName( CString * name );
	cnet * AddNet( CString name, int max_pins, int def_width, int def_via_w, int def_via_hole_w );
	void RemoveNet( cnet * net );
	void RemoveAllNets();
	void AddNetPin( cnet * net, CString * ref_des, CString * pin_name, BOOL set_areas=TRUE );
	void RemoveNetPin( cpart * part, CString * pin_name, BOOL bSetAreas=TRUE );
	void RemoveNetPin( cnet * net, CString * ref_des, CString * pin_name, BOOL bSetAreas=TRUE );
	void RemoveNetPin( cnet * net, int pin_index, BOOL bSetAreas=TRUE );
	void DisconnectNetPin( cpart * part, CString * pin_name, BOOL bSetAreas=TRUE );
	void DisconnectNetPin( cnet * net, CString * ref_des, CString * pin_name, BOOL bSetAreas=TRUE );
	int GetNetPinIndex( cnet * net, CString * ref_des, CString * pin_name );
	int SetNetWidth( cnet * net, int w, int via_w, int via_hole_w );
	void SetNetVisibility( cnet * net, BOOL visible );
	BOOL GetNetVisibility( cnet * net );
	int CheckNetlist( CString * logstr );
	int CheckConnectivity( CString * logstr );
	void HighlightNetConnections( cnet * net );
	void HighlightNet( cnet * net );
	cnet * GetFirstNet();
	cnet * GetNextNet();
	void CancelNextNet();
	void GetWidths( cnet * net, int * w, int * via_w, int * via_hole_w );
	BOOL GetNetBoundaries( CRect * r );

	// functions for connections
	int AddNetConnect( cnet * net, int p1, int p2 );
	int AddNetStub( cnet * net, int p1 );
	int RemoveNetConnect( cnet * net, int ic, BOOL set_areas=TRUE );
	int UnrouteNetConnect( cnet * net, int ic );
	int SetConnectionWidth( cnet * net, int ic, int w, int via_w, int via_hole_w );
	void OptimizeConnections( BOOL bBelowPinCount=FALSE, int pin_count=0, BOOL bVisibleNetsOnly=TRUE );
	int OptimizeConnections( cnet * net, int ic, BOOL bBelowPinCount, int pin_count, BOOL bVisibleNetsOnly=TRUE );
	void OptimizeConnections( cpart * part, BOOL bBelowPinCount, int pin_count, BOOL bVisibleNetsOnly=TRUE );
	void RenumberConnection( cnet * net, int ic );
	void RenumberConnections( cnet * net );
	BOOL TestHitOnConnectionEndPad( int x, int y, cnet * net, int ic, int layer, int dir );
	int TestHitOnAnyPadInNet( int x, int y, int layer, cnet * net );
	void ChangeConnectionPin( cnet * net, int ic, int end_flag,
		cpart * part, CString * pin_name );
	void HighlightConnection( cnet * net, int ic );
	void UndrawConnection( cnet * net, int ic );
	void DrawConnection( cnet * net, int ic );
	void CleanUpConnections( cnet * net, CString * logstr=NULL );
	void CleanUpAllConnections( CString * logstr=NULL );

	// functions for segments
	int AppendSegment( cnet * net, int ic, int x, int y, int layer, int width );
	int InsertSegment( cnet * net, int ic, int iseg, int x, int y, int layer, int width,
						int via_width, int via_hole_width, int dir );
	id  UnrouteSegment( cnet * net, int ic, int iseg );
	void UnrouteSegmentWithoutMerge( cnet * net, int ic, int iseg );
	id MergeUnroutedSegments( cnet * net, int ic );
	int RouteSegment( cnet * net, int ic, int iseg, int layer, int width );
	void RemoveSegment( cnet * net, int ic, int iseg, BOOL bHandleTees=FALSE, BOOL bSetAreaConnections=TRUE );
	int ChangeSegmentLayer( cnet * net, int ic, int iseg, int layer );
	int SetSegmentWidth( cnet * net, int ic, int is, int w, int via_w, int via_hole_w );
	void HighlightSegment( cnet * net, int ic, int iseg );
	int StartMovingSegment( QPainter *painter, cnet * net, int ic, int ivtx,
								   int x, int y, int crosshair, int use_third_segment );
	int StartDraggingSegment( QPainter *painter, cnet * net, int ic, int iseg,
						int x, int y, int layer1, int layer2, int w,
						int layer_no_via, int via_w, int via_hole_w, int dir,
						int crosshair = 1 );
	int CancelDraggingSegment( cnet * net, int ic, int iseg );
	int StartDraggingSegmentNewVertex( QPainter *painter, cnet * net, int ic, int iseg,
								   int x, int y, int layer, int w, int crosshair );
	int CancelDraggingSegmentNewVertex( cnet * net, int ic, int iseg );
	void StartDraggingStub( QPainter *painter, cnet * net, int ic, int iseg,
						int x, int y, int layer1, int w,
						int layer_no_via, int via_w, int via_hole_w,
						int crosshair, int inflection_mode );
	void CancelDraggingStub( cnet * net, int ic, int iseg );
	int CancelMovingSegment( cnet * net, int ic, int ivtx );

	// functions for vias
	int ReconcileVia( cnet * net, int ic, int ivtx );
	int ForceVia( cnet * net, int ic, int ivtx, BOOL set_areas=TRUE );
	int UnforceVia( cnet * net, int ic, int ivtx, BOOL set_areas=TRUE );
	int DrawVia( cnet * net, int ic, int iv );
	void UndrawVia( cnet * net, int ic, int iv );
	void SetViaVisible( cnet * net, int ic, int iv, BOOL visible );

	// functions for vertices
	void HighlightVertex( cnet * net, int ic, int ivtx );
	int StartDraggingVertex( QPainter *painter, cnet * net, int ic, int iseg,
						int x, int y, int cosshair = 1 );
	int CancelDraggingVertex( cnet * net, int ic, int ivtx );
	void StartDraggingEndVertex( QPainter *painter, cnet * net, int ic,
		int ivtx, int crosshair = 1 );
	void CancelDraggingEndVertex( cnet * net, int ic, int ivtx );
	void MoveEndVertex( cnet * net, int ic, int ivtx, int x, int y );
	void MoveVertex( cnet * net, int ic, int ivtx, int x, int y );
	int GetViaConnectionStatus( cnet * net, int ic, int iv, int layer );
	void GetViaPadInfo( cnet * net, int ic, int iv, int layer,
		int * pad_w, int * hole_w, int * connect_status );
	BOOL TestForHitOnVertex( cnet * net, int layer, int x, int y,
		cnet ** hit_net, int * hit_ic, int * hit_iv );

	// functions related to parts
	int RehookPartsToNet( cnet * net );
	void PartAdded( cpart * part );
	int PartMoved( cpart * part );
	int PartFootprintChanged( cpart * part );
	int PartDeleted( cpart * part, BOOL bSetAreas=TRUE );
	int PartDisconnected( cpart * part, BOOL bSetAreas=TRUE );
	void SwapPins( cpart * part1, CString * pin_name1,
						cpart * part2, CString * pin_name2 );
	void PartRefChanged( CString * old_ref_des, CString * new_ref_des );

	// functions for copper areas
	int AddArea( cnet * net, int layer, int x, int y, int hatch );
	void InsertArea( cnet * net, int iarea, int layer, int x, int y, int hatch );
	int AppendAreaCorner( cnet * net, int iarea, int x, int y, int style, BOOL bDraw=TRUE );
	int InsertAreaCorner( cnet * net, int iarea, int icorner,
		int x, int y, int style );
	void MoveAreaCorner( cnet * net, int iarea, int icorner, int x, int y );
	void HighlightAreaCorner( cnet * net, int iarea, int icorner );
	void HighlightAreaSides( cnet * net, int ia );
	CPoint GetAreaCorner( cnet * net, int iarea, int icorner );
	int CompleteArea( cnet * net, int iarea, int style );
	void SetAreaConnections();
	void SetAreaConnections( cnet * net, int iarea );
	void SetAreaConnections( cnet * net );
	void SetAreaConnections( cpart * part );
	BOOL TestPointInArea( cnet * net, int x, int y, int layer, int * iarea );
	int RemoveArea( cnet * net, int iarea );
	void SelectAreaSide( cnet * net, int iarea, int iside );
	void SelectAreaCorner( cnet * net, int iarea, int icorner );
	void SetAreaSideStyle( cnet * net, int iarea, int iside, int style );
	int StartDraggingAreaCorner( QPainter *painter, cnet * net, int iarea, int icorner, int x, int y, int crosshair = 1 );
	int CancelDraggingAreaCorner( cnet * net, int iarea, int icorner );
	int StartDraggingInsertedAreaCorner( QPainter *painter, cnet * net, int iarea, int icorner, int x, int y, int crosshair = 1 );
	int CancelDraggingInsertedAreaCorner( cnet * net, int iarea, int icorner );
	void RenumberAreas( cnet * net );
	int TestAreaPolygon( cnet * net, int iarea );
	int ClipAreaPolygon( cnet * net, int iarea,
		BOOL bMessageBoxArc, BOOL bMessageBoxInt, BOOL bRetainArcs=TRUE );
	int AreaPolygonModified( cnet * net, int iarea, BOOL bMessageBoxArc, BOOL bMessageBoxInt );
	int CombineAllAreasInNet( cnet * net, BOOL bMessageBox, BOOL bUseUtility );
	int TestAreaIntersections( cnet * net, int ia );
	int TestAreaIntersection( cnet * net, int ia1, int ia2 );
	int CombineAreas( cnet * net, int ia1, int ia2 );
	void ApplyClearancesToArea( cnet * net, int ia, int flags,
			int fill_clearance, int min_silkscreen_stroke_wid,
			int thermal_wid, int hole_clearance );

	// I/O  functions
	int WriteNets( CStdioFile * file );
	void ReadNets( CStdioFile * pcb_file, double read_version, int * layers=NULL );
	void ExportNetListInfo( netlist_info * nl );
	void ImportNetListInfo( netlist_info * nl, int flags, CDlgLog * log,
		int def_w, int def_w_v, int def_w_v_h );
	void Copy( CNetList * nl );
	void RestoreConnectionsAndAreas( CNetList * old_nl, int flags, CDlgLog * log=NULL );
	void ReassignCopperLayers( int n_new_layers, int * layer );
	void ImportNetRouting( CString * name, CArray<cnode> * nodes,
		CArray<cpath> * paths, int tolerance, CDlgLog * log=NULL, BOOL bVerbose=TRUE );

	// undo functions
	undo_con * CreateConnectUndoRecord( cnet * net, int icon, BOOL set_areas=TRUE );
	undo_area * CreateAreaUndoRecord( cnet * net, int iarea, int type );
	undo_net * CreateNetUndoRecord( cnet * net );
	static void ConnectUndoCallback( int type, void * ptr, BOOL undo );
	static void AreaUndoCallback( int type, void * ptr, BOOL undo );
	static void NetUndoCallback( int type, void * ptr, BOOL undo );

	// functions for tee_IDs
	void ClearTeeIDs();
	int GetNewTeeID();
	int FindTeeID( int id );
	void RemoveTeeID( int id );
	void AddTeeID( int id );
	// functions for tees and branches
	BOOL FindTeeVertexInNet( cnet * net, int id, int * ic=NULL, int * iv=NULL );
	BOOL FindTeeVertex( int id, cnet ** net, int * ic=NULL, int * iv=NULL );
	int RemoveTee( cnet * net, int id );
	BOOL DisconnectBranch( cnet * net, int ic );
	int RemoveTeeIfNoBranches( cnet * net, int id );
	BOOL TeeViaNeeded( cnet * net, int id );
	BOOL RemoveOrphanBranches( cnet * net, int id, BOOL bRemoveSegs=FALSE );

private:
	CDisplayList * m_dlist;
	CPartList * m_plist;
	int m_layers;	// number of copper layers
	int m_def_w, m_def_via_w, m_def_via_hole_w;
	int m_pos_i;	// index for iterators
	POSITION m_pos[MAX_ITERATORS];	// iterators for nets
	CArray<int> m_tee;
	BOOL m_bSMT_connect;

public:
	int m_annular_ring;
};

#endif // NETLIST_H
