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
class PartList;
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
	CNetList( PartList * plist );
	~CNetList();
	void SetNumCopperLayers( int layers ){ m_layers = layers;}
	void SetWidths( int w, int via_w, int via_hole_w );
	void SetViaAnnularRing( int ring ){ m_annular_ring = ring; }
	void SetSMTconnect( bool bSMTconnect ){ m_bSMT_connect = bSMTconnect; }

	void AddNet(Net* new_net );

	// functions for nets and pins
	void MarkAllNets( int utility );
	void MoveOrigin( int x_off, int y_off );
	Net * GetNetPtrByName( const QString & name );
	void RemoveNet( Net * net );
	void RemoveAllNets();


	int CheckNetlist();
	int CheckConnectivity();

	// WTF is this for?
	//bool GetNetBoundaries( CRect * r );

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

private:
	PartList * m_plist;
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
