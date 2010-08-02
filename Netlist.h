#ifndef NETLIST_H
#define NETLIST_H

#include <QString>
#include <QList>
#include <QHash>
#include "PolyLine.h" 

// Forward declarations
class Net;
class Pin;
class Trace;
class Segment;
class Vertex;
class Part;
class Area;
class CPartList;
class Footprint;



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
	CNetList( CPartList * plist );
	~CNetList();
	void SetNumCopperLayers( int layers ){ m_layers = layers;}
	void SetWidths( int w, int via_w, int via_hole_w );
	void SetViaAnnularRing( int ring ){ m_annular_ring = ring; }
	void SetSMTconnect( bool bSMTconnect ){ m_bSMT_connect = bSMTconnect; }

	void AddNet(Net* new_net );


	// NET methods
	void RemoveNetPin( cnet * net, CString * ref_des, CString * pin_name, bool bSetAreas=true );
	void RemoveNetPin( cnet * net, int pin_index, bool bSetAreas=true );
	void DisconnectNetPin( cnet * net, CString * ref_des, CString * pin_name, bool bSetAreas=true );
	int SetNetWidth( cnet * net, int w, int via_w, int via_hole_w );
	void SetNetVisibility( cnet * net, bool visible );
	bool GetNetVisibility( cnet * net );
	void HighlightNetConnections( cnet * net );
	void HighlightNet( cnet * net );
	int AddNetConnect( cnet * net, int p1, int p2 );
	int AddNetStub( cnet * net, int p1 );
	int RemoveNetConnect( cnet * net, int ic, bool set_areas=true );
	int UnrouteNetConnect( cnet * net, int ic );
	int SetConnectionWidth( cnet * net, int ic, int w, int via_w, int via_hole_w );
	int OptimizeConnections( cnet * net, int ic, bool bBelowPinCount, int pin_count, bool bVisibleNetsOnly=true );
	void RenumberConnection( cnet * net, int ic );
	void RenumberConnections( cnet * net );
	int AppendSegment( cnet * net, int ic, int x, int y, int layer, int width );
	int InsertSegment( cnet * net, int ic, int iseg, int x, int y, int layer, int width,
						int via_width, int via_hole_width, int dir );
	id  UnrouteSegment( cnet * net, int ic, int iseg );
	void UnrouteSegmentWithoutMerge( cnet * net, int ic, int iseg );
	id MergeUnroutedSegments( cnet * net, int ic );
	int RouteSegment( cnet * net, int ic, int iseg, int layer, int width );
	void RemoveSegment( cnet * net, int ic, int iseg, bool bHandleTees=false, bool bSetAreaConnections=true );
	int ChangeSegmentLayer( cnet * net, int ic, int iseg, int layer );
	int SetSegmentWidth( cnet * net, int ic, int is, int w, int via_w, int via_hole_w );
	void HighlightSegment( cnet * net, int ic, int iseg );
	void ChangeConnectionPin( cnet * net, int ic, int end_flag,
		cpart * part, CString * pin_name );
	void HighlightConnection( cnet * net, int ic );
	void UndrawConnection( cnet * net, int ic );
	void DrawConnection( cnet * net, int ic );
	void CleanUpConnections( cnet * net, CString * logstr=NULL );
	bool TestHitOnConnectionEndPad( int x, int y, cnet * net, int ic, int layer, int dir );
	int TestHitOnAnyPadInNet( int x, int y, int layer, cnet * net );
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
	int ForceVia( cnet * net, int ic, int ivtx, bool set_areas=true );
	int UnforceVia( cnet * net, int ic, int ivtx, bool set_areas=true );
	int DrawVia( cnet * net, int ic, int iv );
	void UndrawVia( cnet * net, int ic, int iv );
	void SetViaVisible( cnet * net, int ic, int iv, bool visible );
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
	bool TestForHitOnVertex( cnet * net, int layer, int x, int y,
		cnet ** hit_net, int * hit_ic, int * hit_iv );


	// PIN methods

	// PART methods
	void RemoveNetPin( cpart * part, CString * pin_name, bool bSetAreas=true );
	void DisconnectNetPin( cpart * part, CString * pin_name, bool bSetAreas=true );
	void OptimizeConnections( cpart * part, bool bBelowPinCount, int pin_count, bool bVisibleNetsOnly=true );


	// functions for nets and pins
	void MarkAllNets( int utility );
	void MoveOrigin( int x_off, int y_off );
	cnet * GetNetPtrByName( CString * name );
	void RemoveNet( cnet * net );
	void RemoveAllNets();


	int CheckNetlist( CString * logstr );
	int CheckConnectivity( CString * logstr );

	cnet * GetFirstNet();
	cnet * GetNextNet();
	void CancelNextNet();
	void GetWidths( cnet * net, int * w, int * via_w, int * via_hole_w );
	bool GetNetBoundaries( CRect * r );

	// functions for connections

	void OptimizeConnections( bool bBelowPinCount=false, int pin_count=0, bool bVisibleNetsOnly=true );



	void CleanUpAllConnections( CString * logstr=NULL );

	// functions for segments






	// functions related to parts
	int RehookPartsToNet( cnet * net );
	void PartAdded( cpart * part );
	int PartMoved( cpart * part );
	int PartFootprintChanged( cpart * part );
	int PartDeleted( cpart * part, bool bSetAreas=true );
	int PartDisconnected( cpart * part, bool bSetAreas=true );
	void SwapPins( cpart * part1, CString * pin_name1,
						cpart * part2, CString * pin_name2 );
	void PartRefChanged( CString * old_ref_des, CString * new_ref_des );

	// functions for copper areas
	int AddArea( cnet * net, int layer, int x, int y, int hatch );
	void InsertArea( cnet * net, int iarea, int layer, int x, int y, int hatch );
	int RemoveArea( cnet * net, int iarea );

	// MOVE THIS TO AREA
	void SetAreaConnections();
	void SetAreaConnections( cnet * net, int iarea );
	void SetAreaConnections( cnet * net );
	void SetAreaConnections( cpart * part );

//	bool TestPointInArea( cnet * net, int x, int y, int layer, int * iarea );

//	void SelectAreaSide( cnet * net, int iarea, int iside );
//	void SelectAreaCorner( cnet * net, int iarea, int icorner );
//	void SetAreaSideStyle( cnet * net, int iarea, int iside, int style );
//	int StartDraggingAreaCorner( QPainter *painter, cnet * net, int iarea, int icorner, int x, int y, int crosshair = 1 );
//	int CancelDraggingAreaCorner( cnet * net, int iarea, int icorner );
//	int StartDraggingInsertedAreaCorner( QPainter *painter, cnet * net, int iarea, int icorner, int x, int y, int crosshair = 1 );
//	int CancelDraggingInsertedAreaCorner( cnet * net, int iarea, int icorner );
//	void RenumberAreas( cnet * net );


	int CombineAllAreasInNet( cnet * net, bool bMessageBox, bool bUseUtility );
	int TestAreaIntersections( cnet * net, int ia );

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
		CArray<cpath> * paths, int tolerance, CDlgLog * log=NULL, bool bVerbose=true );

	// undo functions
	undo_con * CreateConnectUndoRecord( cnet * net, int icon, bool set_areas=true );
	undo_area * CreateAreaUndoRecord( cnet * net, int iarea, int type );
	undo_net * CreateNetUndoRecord( cnet * net );
	static void ConnectUndoCallback( int type, void * ptr, bool undo );
	static void AreaUndoCallback( int type, void * ptr, bool undo );
	static void NetUndoCallback( int type, void * ptr, bool undo );

	// functions for tee_IDs
	void ClearTeeIDs();
	int GetNewTeeID();
	int FindTeeID( int id );
	void RemoveTeeID( int id );
	void AddTeeID( int id );
	// functions for tees and branches
	bool FindTeeVertexInNet( cnet * net, int id, int * ic=NULL, int * iv=NULL );
	bool FindTeeVertex( int id, cnet ** net, int * ic=NULL, int * iv=NULL );
	int RemoveTee( cnet * net, int id );
	bool DisconnectBranch( cnet * net, int ic );
	int RemoveTeeIfNoBranches( cnet * net, int id );
	bool TeeViaNeeded( cnet * net, int id );
	bool RemoveOrphanBranches( cnet * net, int id, bool bRemoveSegs=false );

private:
	CPartList * m_plist;
	int m_layers;	// number of copper layers
	int m_def_w, m_def_via_w, m_def_via_hole_w;
	int m_pos_i;	// index for iterators
	POSITION m_pos[MAX_ITERATORS];	// iterators for nets
	CArray<int> m_tee;
	bool m_bSMT_connect;
	QHash<QString,Net*> m_map;	// map net names to pointers
	int m_annular_ring;
};

#endif // NETLIST_H
