// PartList.cpp : implementation of class CPartList
//
// this is a linked-list of parts on a PCB board
//
#include <math.h>

#define PL_MAX_SIZE		5000		// default max. size 

// globals
BOOL g_bShow_header_28mil_hole_warning = TRUE;	
BOOL g_bShow_SIP_28mil_hole_warning = TRUE;	

//******** constructors and destructors *********

CPartList::CPartList( CDisplayList * dlist, SMFontUtil * fontutil ) 
{
	m_start.prev = 0;		// dummy first element in list
	m_start.next = &m_end;
	m_end.next = 0;			// dummy last element in list
	m_end.prev = &m_start;
	m_max_size = PL_MAX_SIZE;	// size limit
	m_size = 0;					// current size
	m_dlist = dlist;
	m_fontutil = fontutil;
	m_footprint_cache_map = NULL;
}

CPartList::~CPartList()
{
	// traverse list, removing all parts
	while( m_end.prev != &m_start )
		Remove( m_end.prev );
}

// Create new empty part and add to end of list
// return pointer to element created.
//
cpart * CPartList::Add()
{
	if(m_size >= m_max_size )
	{
		AfxMessageBox( "Maximum number of parts exceeded" );
		return 0;
	}

	// create new instance and link into list
	cpart * part = new cpart;
	part->prev = m_end.prev;
	part->next = &m_end;
	part->prev->next = part;
	part->next->prev = part;

	return part;
}

// Create new part, add to end of list, set part data 
// return pointer to element created.
//
cpart * CPartList::Add( CShape * shape, CString * ref_des, CString * package, 
							int x, int y, int side, int angle, int visible, int glued )
{
	if(m_size >= m_max_size )
	{
		AfxMessageBox( "Maximum number of parts exceeded" );
		return 0;
	}

	// create new instance and link into list
	cpart * part = Add();
	// set data
	SetPartData( part, shape, ref_des, package, x, y, side, angle, visible, glued );
	return part;
}



void CPartList:: HighlightAllPadsOnNet( cnet * net )
{
	cpart * part = GetFirstPart();
	while( part )
	{
		if( part->shape )
		{
			for( int ip=0; ip<part->shape->GetNumPins(); ip++ )
			{
				if( net == part->pin[ip].net )
					SelectPad( part, ip );
			}
		}
		part = GetNextPart( part );
	}
}



// get bounding rectangle of parts
// return 0 if no parts found, else return 1
//
int CPartList::GetPartBoundaries( CRect * part_r )
{
	int min_x = INT_MAX;
	int max_x = INT_MIN;
	int min_y = INT_MAX;
	int max_y = INT_MIN;
	int parts_found = 0;
	// iterate
	cpart * part = m_start.next;
	while( part->next != 0 )
	{
		if( part->dl_sel )
		{
			int x = m_dlist->Get_x( part->dl_sel );
			int y = m_dlist->Get_y( part->dl_sel );
			max_x = max( x, max_x);
			min_x = min( x, min_x);
			max_y = max( y, max_y);
			min_y = min( y, min_y);
			x = m_dlist->Get_xf( part->dl_sel );
			y = m_dlist->Get_yf( part->dl_sel );
			max_x = max( x, max_x);
			min_x = min( x, min_x);
			max_y = max( y, max_y);
			min_y = min( y, min_y);
			parts_found = 1;
		}
		if( part->dl_ref_sel )
		{
			int x = m_dlist->Get_x( part->dl_ref_sel );
			int y = m_dlist->Get_y( part->dl_ref_sel );
			max_x = max( x, max_x);
			min_x = min( x, min_x);
			max_y = max( y, max_y);
			min_y = min( y, min_y);
			x = m_dlist->Get_xf( part->dl_ref_sel );
			y = m_dlist->Get_yf( part->dl_ref_sel );
			max_x = max( x, max_x);
			min_x = min( x, min_x);
			max_y = max( y, max_y);
			min_y = min( y, min_y);
			parts_found = 1;
		}
		part = part->next;
	}
	part_r->left = min_x;
	part_r->right = max_x;
	part_r->bottom = min_y;
	part_r->top = max_y;
	return parts_found;
}

// Get pointer to part in part_list with given ref
//
cpart * CPartList::GetPart( LPCTSTR ref_des )
{
	// find element with given ref_des, return pointer to element
	cpart * part = m_start.next;
	while( part->next != 0 )
	{
		if(  part->ref_des == ref_des  )
			return part;
		part = part->next;
	}
	return NULL;	// if unable to find part
}

// Iterate through parts
//
cpart * CPartList::GetFirstPart()
{
	cpart * p = m_start.next;
	if( p->next )
		return p;
	else
		return NULL;
}

cpart * CPartList::GetNextPart( cpart * part )
{
	cpart * p = part->next;
	if( !p )
		return NULL;
	if( !p->next )
		return NULL;
	else
		return p;
}

// get number of times a particular shape is used
//
int CPartList::GetNumFootprintInstances( CShape * shape )
{
	int n = 0;

	cpart * part = m_start.next;
	while( part->next != 0 )
	{
		if(  part->shape == shape  )
			n++;
		part = part->next;
	}
	return n;
}

// Purge unused footprints from cache
//
void CPartList::PurgeFootprintCache()
{
	POSITION pos;
	CString key;
	void * ptr;

	if( !m_footprint_cache_map )
		ASSERT(0);

	for( pos = m_footprint_cache_map->GetStartPosition(); pos != NULL; )
	{
		m_footprint_cache_map->GetNextAssoc( pos, key, ptr );
		CShape * shape = (CShape*)ptr;
		if( GetNumFootprintInstances( shape ) == 0 )
		{
			// purge this footprint
			delete shape;
			m_footprint_cache_map->RemoveKey( key );
		}
	}
}

// Remove part from list and delete it
//
int CPartList::Remove( cpart * part )
{
	// delete all entries in display list
	UndrawPart( part );

	// remove links to this element
	part->next->prev = part->prev;
	part->prev->next = part->next;
	// destroy part
	m_size--;
	delete( part );

	return 0;
}

// Remove all parts from list
//
void CPartList::RemoveAllParts()
{
	// traverse list, removing all parts
	while( m_end.prev != &m_start )
		Remove( m_end.prev );
}

// Set utility flag for all parts
//
void CPartList::MarkAllParts( int mark )
{
	cpart * part = GetFirstPart();
	while( part )
	{
		part->utility = mark;
		part = GetNextPart( part );
	}
}



// the footprint was modified, apply to all parts using it
//
void CPartList::FootprintChanged( CShape * shape )
{
	// find all parts with given footprint and update them
	cpart * part = m_start.next;
	while( part->next != 0 )
	{
		if( part->shape )
		{
			if(  part->shape->m_name == shape->m_name  )
			{
				PartFootprintChanged( part, shape );
			}
		}
		part = part->next;
	}
}

// the ref text height and width were modified, apply to all parts using it
//
void CPartList::RefTextSizeChanged( CShape * shape )
{
	// find all parts with given shape and update them
	cpart * part = m_start.next;
	while( part->next != 0 )
	{
		if(  part->shape->m_name == shape->m_name  )
		{
			ResizeRefText( part, shape->m_ref_size, shape->m_ref_w );
		}
		part = part->next;
	}
}



// create part from string
//
cpart * CPartList::AddFromString( CString * str )
{
	CShape * s = NULL;
	CString in_str, key_str;
	CArray<CString> p;
	int pos = 0;
	int len = str->GetLength();
	int np;
	CString ref_des;
	BOOL ref_vis = FALSE;
	int ref_size = 0;
	int ref_width = 0;
	int ref_angle = 0;
	int ref_xi = 0;
	int ref_yi = 0;
	CString value;
	BOOL value_vis = FALSE;
	int value_size = 0;
	int value_width = 0;
	int value_angle = 0;
	int value_xi = 0;
	int value_yi = 0;
	CString package;
	int x;
	int y;
	int side;
	int angle;
	int glued;
	cpart * part = Add();

	// so we only draw once
	CDisplayList * old_dlist = m_dlist;
	m_dlist = NULL;

	in_str = str->Tokenize( "\n", pos );
	while( in_str != "" )
	{
		np = ParseKeyString( &in_str, &key_str, &p );
		if( key_str == "ref" )
		{
			ref_des = in_str.Right( in_str.GetLength()-4 );
			ref_des.Trim();
			ref_des = ref_des.Left(MAX_REF_DES_SIZE);
		}
		else if( key_str == "part" )
		{
			ref_des = in_str.Right( in_str.GetLength()-5 );
			ref_des.Trim();
			ref_des = ref_des.Left(MAX_REF_DES_SIZE);
		}
		else if( np >= 6 && key_str == "ref_text" )
		{
			ref_size = my_atoi( &p[0] );
			ref_width = my_atoi( &p[1] );
			ref_angle = my_atoi( &p[2] );
			ref_xi = my_atoi( &p[3] );
			ref_yi = my_atoi( &p[4] );
			if( np > 6 )
				ref_vis = my_atoi( &p[5] );
			else
			{
				if( ref_size )
					ref_vis = TRUE;
				else
					ref_vis = FALSE;
			}
		}
		else if( np >= 7 && key_str == "value" )
		{
			value = p[0];
			value_size = my_atoi( &p[1] );
			value_width = my_atoi( &p[2] );
			value_angle = my_atoi( &p[3] );
			value_xi = my_atoi( &p[4] );
			value_yi = my_atoi( &p[5] );
			if( np > 7 )
				value_vis = my_atoi( &p[6] );
			else
			{
				if( value_size )
					value_vis = TRUE;
				else
					value_vis = FALSE;
			}
		}
		else if( key_str == "package" )
		{
			if( np >= 2 )
				package = p[0];
			else
				package = "";
			package = package.Left(CShape::MAX_NAME_SIZE);
		}
		else if( np >= 2 && key_str == "shape" )
		{
			// lookup shape in cache
			s = NULL;
			void * ptr;
			CString name = p[0];
			name = name.Left(CShape::MAX_NAME_SIZE);
			int err = m_footprint_cache_map->Lookup( name, ptr );
			if( err )
			{
				// found in cache
				s = (CShape*)ptr; 
			}
		}
		else if( key_str == "pos" )
		{
			if( np >= 6 )
			{
				x = my_atoi( &p[0] );
				y = my_atoi( &p[1] );
				side = my_atoi( &p[2] );
				angle = my_atoi( &p[3] );
				glued = my_atoi( &p[4] );
			}
			else
			{
				x = 0;
				y = 0;
				side = 0;
				angle = 0;
				glued = 0;
			}
		}
		in_str = str->Tokenize( "\n", pos );
	}
	SetPartData( part, s, &ref_des, &package, x, y, side, angle, 1, glued );
	SetValue( part, &value, value_xi, value_yi, value_angle, value_size, value_width, value_vis );
	if( part->shape ) 
	{
		part->m_ref_xi = ref_xi;
		part->m_ref_yi = ref_yi;
		part->m_ref_angle = ref_angle;
		ResizeRefText( part, ref_size, ref_width, ref_vis );
	}
	m_dlist = old_dlist;
	DrawPart( part );
	return part;
}

// read partlist
//
int CPartList::ReadParts( CStdioFile * pcb_file )
{
	int pos, err;
	CString in_str, key_str;
	CArray<CString> p;

	// find beginning of [parts] section
	do
	{
		err = pcb_file->ReadString( in_str );
		if( !err )
		{
			// error reading pcb file
			CString mess;
			mess.Format( "Unable to find [parts] section in file" );
			AfxMessageBox( mess );
			return 0;
		}
		in_str.Trim();
	}
	while( in_str != "[parts]" );

	// get each part in [parts] section
	while( 1 )
	{
		pos = pcb_file->GetPosition();
		err = pcb_file->ReadString( in_str );
		if( !err )
		{
			CString * err_str = new CString( "unexpected EOF in project file" );
			throw err_str;
		}
		in_str.Trim();
		if( in_str[0] == '[' && in_str != "[parts]" )
		{
			pcb_file->Seek( pos, CFile::begin );
			break;		// next section, exit
		}
		else if( in_str.Left(4) == "ref:" || in_str.Left(5) == "part:" )
		{
			CString str;
			do
			{
				str.Append( in_str );
				str.Append( "\n" );
				pos = pcb_file->GetPosition();
				err = pcb_file->ReadString( in_str );
				if( !err )
				{
					CString * err_str = new CString( "unexpected EOF in project file" );
					throw err_str;
				}
				in_str.Trim();
			} while( (in_str.Left(4) != "ref:" && in_str.Left(5) != "part:" )
						&& in_str[0] != '[' );
			pcb_file->Seek( pos, CFile::begin );

			// now add part to partlist
			cpart * part = AddFromString( &str );
		}
	}
	return 0;
}

// write all parts and footprints to file
//
int CPartList::WriteParts( CStdioFile * file )
{
	CMapStringToPtr shape_map;
	cpart * el = m_start.next;
	CString line;
	CString key;
	try
	{
		// now write all parts
		line.Format( "[parts]\n\n" );
		file->WriteString( line );
		el = m_start.next;
		while( el->next != 0 )
		{
			// test
			CString test;
			SetPartString( el, &test );
			file->WriteString( test );
			el = el->next;
		}
		
	}
	catch( CFileException * e )
	{
		CString str;
		if( e->m_lOsError == -1 )
			str.Format( "File error: %d\n", e->m_cause );
		else
			str.Format( "File error: %d %ld (%s)\n", e->m_cause, e->m_lOsError,
			_sys_errlist[e->m_lOsError] );
		return 1;
	}

	return 0;
}

// utility function to rotate a point clockwise about another point
// currently, angle must be 0, 90, 180 or 270
//
int CPartList::RotatePoint( CPoint *p, int angle, CPoint org )
{
	CRect tr;
	if( angle == 90 )
	{
		int tempy = org.y + (org.x - p->x);
		p->x = org.x + (p->y - org.y);
		p->y = tempy;
	}
	else if( angle > 90 )
	{
		for( int i=0; i<(angle/90); i++ )
			RotatePoint( p, 90, org );
	}
	return PL_NOERR;
}

// utility function to rotate a rectangle clockwise about a point
// currently, angle must be 0, 90, 180 or 270
// assumes that (r->right) > (r->left), (r->top) > (r->bottom)
//
int CPartList::RotateRect( CRect *r, int angle, CPoint org )
{
	CRect tr;
	if( angle == 90 )
	{
		tr.left = org.x + (r->bottom - org.y);
		tr.right = org.x + (r->top - org.y);
		tr.bottom = org.y + (org.x - r->right);
		tr.top = org.y + (org.x - r->left);
	}
	else if( angle > 90 )
	{
		tr = *r;
		for( int i=0; i<(angle/90); i++ )
			RotateRect( &tr, 90, org );
	}
	*r = tr;
	return PL_NOERR;
}

// export part list data into partlist_info structure for editing in dialog
// if test_part != NULL, returns index of test_part in partlist_info
//
int CPartList::ExportPartListInfo( partlist_info * pl, cpart * test_part )
{
	// traverse part list to find number of parts
	int ipart = -1;
	int nparts = 0;
	cpart * part = m_start.next;
	while( part->next != 0 )
	{
		nparts++;
		part = part->next;
	}
	// now make struct
	pl->SetSize( nparts );
	int i = 0;
	part = m_start.next;
	while( part->next != 0 )
	{
		if( part == test_part )
			ipart = i;
		(*pl)[i].part = part;
		(*pl)[i].shape = part->shape;
		(*pl)[i].bShapeChanged = FALSE;
		(*pl)[i].ref_des = part->ref_des;
		if( part->shape )
		{
			(*pl)[i].ref_size = part->m_ref_size;
			(*pl)[i].ref_width = part->m_ref_w;
		}
		else
		{
			(*pl)[i].ref_size = 0;
			(*pl)[i].ref_width = 0;
		}
		(*pl)[i].package = part->package;
		(*pl)[i].value = part->value;
		(*pl)[i].value_vis = part->m_value_vis;
		(*pl)[i].x = part->x;
		(*pl)[i].y = part->y;
		(*pl)[i].angle = part->angle;
		(*pl)[i].side = part->side;
		(*pl)[i].deleted = FALSE;
		(*pl)[i].bOffBoard = FALSE;
		i++;
		part = part->next;
	}
	return ipart;
}

// import part list data from struct partlist_info
//
void CPartList::ImportPartListInfo( partlist_info * pl, int flags, CDlgLog * log )
{
	CString mess; 

	// undraw all parts and disable further drawing
	CDisplayList * old_dlist = m_dlist;
	if( m_dlist )
	{
		cpart * part = GetFirstPart();
		while( part )
		{
			UndrawPart( part );
			part = GetNextPart( part );
		}
	}
	m_dlist = NULL;		

	// grid for positioning parts off-board
	int pos_x = 0;
	int pos_y = 0;
	enum { GRID_X = 100, GRID_Y = 50 };
	BOOL * grid = (BOOL*)calloc( GRID_X*GRID_Y, sizeof(BOOL) );
	int grid_num = 0;

	// first, look for parts in project whose ref_des has been changed
	for( int i=0; i<pl->GetSize(); i++ )
	{
		part_info * pi = &(*pl)[i];
		if( pi->part )
		{
			if( pi->ref_des != pi->part->ref_des )
			{
				m_nlist->PartRefChanged( &pi->part->ref_des, &pi->ref_des );
				pi->part->ref_des = pi->ref_des;
			}
		}
	}

	// now find parts in project that are not in partlist_info
	// loop through all parts in project
	cpart * part = m_start.next;
	while( part->next != 0 )
	{
		// loop through the partlist_info array
		BOOL bFound = FALSE;
		part->bPreserve = FALSE;
		for( int i=0; i<pl->GetSize(); i++ )
		{
			part_info * pi = &(*pl)[i];
			if( pi->ref_des == part->ref_des )
			{
				// part exists in partlist_info
				bFound = TRUE;
				break;
			}
		}
		cpart * next_part = part->next;
		if( !bFound )
		{
			// part in project but not in partlist_info
			if( flags & KEEP_PARTS_AND_CON )
			{
				// set flag to preserve this part
				part->bPreserve = TRUE;
				if( log )
				{
					mess.Format( "  Keeping part %s and connections\r\n", part->ref_des );
					log->AddLine( mess );
				}
			}
			else if( flags & KEEP_PARTS_NO_CON )
			{
				// keep part but remove connections from netlist
				if( log )
				{
					mess.Format( "  Keeping part %s but removing connections\r\n", part->ref_des );
					log->AddLine( mess );
				}
				m_nlist->PartDeleted( part );
			}
			else
			{
				// remove part
				if( log )
				{
					mess.Format( "  Removing part %s\r\n", part->ref_des );
					log->AddLine( mess );
				}
				m_nlist->PartDeleted( part );
				Remove( part );
			}
		}
		part = next_part;
	}

	// loop through partlist_info array, changing partlist as necessary
	for( int i=0; i<pl->GetSize(); i++ )
	{
		part_info * pi = &(*pl)[i];
		if( pi->part == 0 && pi->deleted )
		{
			// new part was added but then deleted, ignore it
			continue;
		}
		if( pi->part != 0 && pi->deleted )
		{
			// old part was deleted, remove it
			m_nlist->PartDisconnected( pi->part );
			Remove( pi->part );
			continue;
		}

		if( pi->part == 0 )
		{
			// the partlist_info does not include a pointer to an existing part
			// the part might not exist in the project, or we are importing a netlist file
			cpart * old_part = GetPart( pi->ref_des );
			if( old_part )
			{
				// an existing part has the same ref_des as the new part
				if( old_part->shape )
				{
					// the existing part has a footprint
					// see if the incoming package name matches the old package or footprint
					if( (flags & KEEP_FP) 
						|| (pi->package == "") 
						|| (pi->package == old_part->package)
						|| (pi->package == old_part->shape->m_name) )
					{
						// use footprint and parameters from existing part
						pi->part = old_part;
						pi->ref_size = old_part->m_ref_size; 
						pi->ref_width = old_part->m_ref_w;
						pi->value = old_part->value;
						pi->value_vis = old_part->m_value_vis;
						pi->x = old_part->x; 
						pi->y = old_part->y;
						pi->angle = old_part->angle;
						pi->side = old_part->side;
						pi->shape = old_part->shape;
					}
					else if( pi->shape )
					{
						// use new footprint, but preserve position
						pi->ref_size = old_part->m_ref_size; 
						pi->ref_width = old_part->m_ref_w;
						pi->value = old_part->value;
						pi->value_vis = old_part->m_value_vis;
						pi->x = old_part->x; 
						pi->y = old_part->y;
						pi->angle = old_part->angle;
						pi->side = old_part->side;
						pi->part = old_part;
						pi->bShapeChanged = TRUE;
						if( log && old_part->shape->m_name != pi->package )
						{
							mess.Format( "  Changing footprint of part %s from \"%s\" to \"%s\"\r\n", 
								old_part->ref_des, old_part->shape->m_name, pi->shape->m_name );
							log->AddLine( mess );
						}
					}
					else
					{
						// new part does not have footprint, remove old part
						if( log && old_part->shape->m_name != pi->package )
						{
							mess.Format( "  Changing footprint of part %s from \"%s\" to \"%s\" (not found)\r\n", 
								old_part->ref_des, old_part->shape->m_name, pi->package );
							log->AddLine( mess );
						}
						m_nlist->PartDisconnected( old_part );
						Remove( old_part );
					}
				}
				else
				{
					// remove old part (which did not have a footprint)
					if( log && old_part->package != pi->package )
					{
						mess.Format( "  Changing footprint of part %s from \"%s\" to \"%s\"\r\n", 
							old_part->ref_des, old_part->package, pi->package );
						log->AddLine( mess );
					}
					m_nlist->PartDisconnected( old_part );
					Remove( old_part );
				}
			}
		}

		if( pi->part )
		{
			if( pi->part->shape != pi->shape || pi->bShapeChanged == TRUE )
			{
				// old part exists, but footprint was changed
				if( pi->part->shape == NULL )
				{
					// old part did not have a footprint before, so remove it
					// and treat as new part
					m_nlist->PartDisconnected( pi->part );
					Remove( pi->part );
					pi->part = NULL;
				}
			}
		}

		if( pi->part == 0 )
		{
			// new part is being imported (with or without footprint)
			if( pi->shape && pi->bOffBoard )
			{
				// place new part offboard, using grid 
				int ix, iy;	// grid indices
				// find size of part in 100 mil units
				BOOL OK = FALSE;
				int w = abs( pi->shape->m_sel_xf - pi->shape->m_sel_xi )/(100*PCBU_PER_MIL)+2;
				int h = abs( pi->shape->m_sel_yf - pi->shape->m_sel_yi )/(100*PCBU_PER_MIL)+2;
				// now find space in grid for part
				for( ix=0; ix<GRID_X; ix++ )
				{
					iy = 0;
					while( iy < (GRID_Y - h) )
					{
						if( !grid[ix+GRID_X*iy] )
						{
							// see if enough space
							OK = TRUE;
							for( int iix=ix; iix<(ix+w); iix++ )
								for( int iiy=iy; iiy<(iy+h); iiy++ )
									if( grid[iix+GRID_X*iiy] )
										OK = FALSE;
							if( OK )
								break;
						}
						iy++;
					}
					if( OK )
						break;
				}
				if( OK )
				{
					// place part
					pi->side = 0;
					pi->angle = 0;
					if( grid_num == 0 )
					{
						// first grid, to left and above origin
						pi->x = -(ix+w)*100*PCBU_PER_MIL;
						pi->y = iy*100*PCBU_PER_MIL;
					}
					else if( grid_num == 1 )
					{
						// second grid, to left and below origin
						pi->x = -(ix+w)*100*PCBU_PER_MIL;
						pi->y = -(iy+h)*100*PCBU_PER_MIL;
					}
					else if( grid_num == 2 )
					{
						// third grid, to right and below origin
						pi->x = ix*100*PCBU_PER_MIL;
						pi->y = -(iy+h)*100*PCBU_PER_MIL;
					}
					// remove space in grid
					for( int iix=ix; iix<(ix+w); iix++ )
						for( int iiy=iy; iiy<(iy+h); iiy++ )
							grid[iix+GRID_X*iiy] = TRUE;
				}
				else
				{
					// fail, go to next grid
					if( grid_num == 2 )
						ASSERT(0);		// ran out of grids
					else
					{
						// zero grid
						for( int j=0; j<GRID_Y; j++ )
							for( int i=0; i<GRID_X; i++ )
								grid[j*GRID_X+i] = FALSE;
						grid_num++;
					}
				}
				// now offset for part origin
				pi->x -= pi->shape->m_sel_xi;
				pi->y -= pi->shape->m_sel_yi;
			}
			// now place part
			cpart * part = Add( pi->shape, &pi->ref_des, &pi->package, pi->x, pi->y,
				pi->side, pi->angle, TRUE, FALSE );
			if( part->shape )
			{
				ResizeRefText( part, pi->ref_size, pi->ref_width );
				SetValue( part, &pi->value, 
					part->shape->m_value_xi, 
					part->shape->m_value_yi,
					part->shape->m_value_angle, 
					part->shape->m_value_size, 
					part->shape->m_value_w,
					pi->value_vis );
			}
			else
				SetValue( part, &pi->value, 0, 0, 0, 0, 0 );
			m_nlist->PartAdded( part );
		}
		else
		{
			// part existed before but may have been modified
			if( pi->part->package != pi->package )
			{
				// package changed
				pi->part->package = pi->package;
			}
			if( pi->part->value != pi->value )
			{
				// value changed, keep size and position
				SetValue( pi->part, &pi->value, 
					pi->part->shape->m_value_xi, 
					pi->part->shape->m_value_yi,
					pi->part->shape->m_value_angle, 
					pi->part->shape->m_value_size, 
					pi->part->shape->m_value_w );

			}
			if( pi->part->m_value_vis != pi->value_vis )
			{
				// value visibility changed
				pi->part->m_value_vis = pi->value_vis;
			}
			if( pi->part->shape != pi->shape || pi->bShapeChanged == TRUE )
			{
				// footprint was changed
				if( pi->part->shape == NULL )
				{
					ASSERT(0);	// should never get here
				}
				else if( pi->shape && !(flags & KEEP_FP) )
				{
					// change footprint to new one
					PartFootprintChanged( pi->part, pi->shape );
					ResizeRefText( pi->part, pi->ref_size, pi->ref_width );
//** 420					m_nlist->PartFootprintChanged( part );
//** 420					m_nlist->PartMoved( pi->part );
				}
			}
			if( pi->x != pi->part->x 
				|| pi->y != pi->part->y
				|| pi->angle != pi->part->angle
				|| pi->side != pi->part->side )
			{
				// part was moved
				Move( pi->part, pi->x, pi->y, pi->angle, pi->side );
				m_nlist->PartMoved( pi->part );
			}
		}
	}
	// PurgeFootprintCache();
	free( grid );

	// redraw partlist
	m_dlist = old_dlist;
	part = GetFirstPart();
	while( part )
	{
		DrawPart( part );
		part = GetNextPart( part );
	}
}

// Design rule check
//
void CPartList::DRC( CDlgLog * log, int copper_layers, 
					int units, BOOL check_unrouted,
					CArray<CPolyLine> * board_outline,
					DesignRules * dr, DRErrorList * drelist )
{
	CString d_str, x_str, y_str;
	CString str;
	CString str2;
	long nerrors = 0;

	// iterate through parts, checking pads and setting DRC params
	str.Format( "Checking parts:\r\n" );
	if( log )
		log->AddLine( str );
	cpart * part = GetFirstPart();
	while( part )
	{
		CShape * s = part->shape;
		if( s )
		{
			// set DRC params for part
			part->hole_flag = FALSE;
			part->min_x = INT_MAX;
			part->max_x = INT_MIN;
			part->min_y = INT_MAX;
			part->max_y = INT_MIN;
			part->layers = 0;

			// iterate through pins in test_part
			for( int ip=0; ip<s->GetNumPins(); ip++ )
			{
				drc_pin * drp = &part->pin[ip].drc;
				drp->hole_size = 0;
				drp->min_x = INT_MAX;
				drp->max_x = INT_MIN;
				drp->min_y = INT_MAX;
				drp->max_y = INT_MIN;
				drp->max_r = INT_MIN;
				drp->layers = 0;

				id id1 = part->m_id;
				id1.st = ID_PAD;
				id1.i = ip;

				// iterate through copper layers
				for( int il=0; il<copper_layers; il++ )
				{
					int layer = LAY_TOP_COPPER + il;
					int layer_bit = 1<<il;

					// get test pad info
					int x, y, w, l, r, type, hole, connect, angle;
					cnet * net;
					BOOL bPad = GetPadDrawInfo( part, ip, layer, 0, 0, 0, 0,
						&type, &x, &y, &w, &l, &r, &hole, &angle,
						&net, &connect );
					if( bPad )
					{
						// pad or hole present
						if( hole )
						{
							drp->hole_size = hole;
							drp->min_x = min( drp->min_x, x - hole/2 );
							drp->max_x = max( drp->max_x, x + hole/2 );
							drp->min_y = min( drp->min_y, y - hole/2 );
							drp->max_y = max( drp->max_y, y + hole/2 );
							drp->max_r = max( drp->max_r, hole/2 );
							part->min_x = min( part->min_x, x - hole/2 );
							part->max_x = max( part->max_x, x + hole/2 );
							part->min_y = min( part->min_y, y - hole/2 );
							part->max_y = max( part->max_y, y + hole/2 );
							part->hole_flag = TRUE;
							// test clearance to board edge
							for( int ib=0; ib<board_outline->GetSize(); ib++ )
							{
								CPolyLine * b = &(*board_outline)[ib];
								for( int ibc=0; ibc<b->GetNumCorners(); ibc++ )
								{
									int x1 = b->GetX(ibc);
									int y1 = b->GetY(ibc);
									int x2 = b->GetX(0);
									int y2 = b->GetY(0);
									if( ibc != b->GetNumCorners()-1 )
									{
										x2 = b->GetX(ibc+1);
										y2 = b->GetY(ibc+1);
									}
									// for now, only works for straight board edge segments
									if( b->GetSideStyle(ibc) == CPolyLine::STRAIGHT )
									{
										int d = ::GetClearanceBetweenSegmentAndPad( x1, y1, x2, y2, 0,
											PAD_ROUND, x, y, hole, 0, 0, 0 );
										if( d < dr->board_edge_copper )
										{
											// BOARDEDGE_PADHOLE error
											::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &x_str, x, units, FALSE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &y_str, y, units, FALSE, TRUE, TRUE, 1 );
											str.Format( "%ld: %s.%s pad hole to board edge = %s, x=%s, y=%s\r\n",  
												nerrors+1, part->ref_des, s->m_padstack[ip].name, d_str, x_str, y_str );
											DRError * dre = drelist->Add( nerrors, DRError::BOARDEDGE_PADHOLE, &str,
												&part->ref_des, NULL, id1, id1, x, y, 0, 0, w+20*NM_PER_MIL, 0 );
											if( dre )
											{
												nerrors++;
												if( log )
													log->AddLine( str );
											}
										}
									}
								}
							}
						}
						if( type != PAD_NONE )
						{
							int wid = w;
							int len = wid;
							if( type == PAD_RECT || type == PAD_RRECT || type == PAD_OVAL )
								len = l;
							if( angle == 90 )
							{
								wid = len;
								len = w;
							}
							drp->min_x = min( drp->min_x, x - len/2 );
							drp->max_x = max( drp->max_x, x + len/2 );
							drp->min_y = min( drp->min_y, y - wid/2 );
							drp->max_y = max( drp->max_y, y + wid/2 );
							drp->max_r = max( drp->max_r, Distance( 0, 0, len/2, wid/2 ) );
							part->min_x = min( part->min_x, x - len/2 );
							part->max_x = max( part->max_x, x + len/2 );
							part->min_y = min( part->min_y, y - wid/2 );
							part->max_y = max( part->max_y, y + wid/2 );
							drp->layers |= layer_bit;
							part->layers |= layer_bit;
							if( hole && part->pin[ip].net )
							{
								// test annular ring
								int d = (w - hole)/2;
								if( type == PAD_RECT || type == PAD_RRECT || type == PAD_OVAL )
									d = (min(w,l) - hole)/2;
								if( d < dr->annular_ring_pins )
								{
									// RING_PAD
									::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
									::MakeCStringFromDimension( &x_str, x, units, FALSE, TRUE, TRUE, 1 );
									::MakeCStringFromDimension( &y_str, y, units, FALSE, TRUE, TRUE, 1 );
									str.Format( "%ld: %s.%s annular ring = %s, x=%s, y=%s\r\n",  
										nerrors+1, part->ref_des, s->m_padstack[ip].name, d_str, x_str, y_str );
									DRError * dre = drelist->Add( nerrors, DRError::RING_PAD, &str,
										&part->ref_des, NULL, id1, id1, x, y, 0, 0, w+20*NM_PER_MIL, 0 );
									if( dre )
									{
										nerrors++;
										if( log )
											log->AddLine( str );
									}
								}
							}
							// test clearance to board edge
							for( int ib=0; ib<board_outline->GetSize(); ib++ )
							{
								CPolyLine * b = &(*board_outline)[ib];
								for( int ibc=0; ibc<b->GetNumCorners(); ibc++ )
								{
									int x1 = b->GetX(ibc);
									int y1 = b->GetY(ibc);
									int x2 = b->GetX(0);
									int y2 = b->GetY(0);
									if( ibc != b->GetNumCorners()-1 )
									{
										x2 = b->GetX(ibc+1);
										y2 = b->GetY(ibc+1);
									}
									// for now, only works for straight board edge segments
									if( b->GetSideStyle(ibc) == CPolyLine::STRAIGHT )
									{
										int d = ::GetClearanceBetweenSegmentAndPad( x1, y1, x2, y2, 0,
											type, x, y, w, l, r, angle );
										if( d < dr->board_edge_copper )
										{
											// BOARDEDGE_PAD error
											::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &x_str, x, units, FALSE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &y_str, y, units, FALSE, TRUE, TRUE, 1 );
											str.Format( "%ld: %s.%s pad to board edge = %s, x=%s, y=%s\r\n",  
												nerrors+1, part->ref_des, s->m_padstack[ip].name, d_str, x_str, y_str );
											DRError * dre = drelist->Add( nerrors, DRError::BOARDEDGE_PAD, &str,
												&part->ref_des, NULL, id1, id1, x, y, 0, 0, w+20*NM_PER_MIL, 0 );
											if( dre )
											{
												nerrors++;
												if( log )
													log->AddLine( str );
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		part = GetNextPart( part );
	}

	// iterate through parts again, checking against all other parts
	for( cpart * t_part=GetFirstPart(); t_part; t_part=GetNextPart(t_part) )
	{
		CShape * t_s = t_part->shape;
		if( t_s )
		{
			// now iterate through parts that follow in the partlist
			for( cpart * part=GetNextPart(t_part); part; part=GetNextPart(part) )
			{
				CShape * s = part->shape;
				if( s )
				{
					// now see if part and t_part pads might intersect
					// get max. clearance violation
					int clr = max( dr->pad_pad, dr->hole_copper );
					clr = max( clr, dr->hole_hole );
					// see if pads on same layers
					if( !(part->layers & t_part->layers) )
					{
						// no pads on same layers,check for holes
						if( !part->hole_flag && !t_part->hole_flag ) 
							continue;	// no, go to next part
					}

					// now check for clearance of rectangles
					if( part->min_x - t_part->max_x > clr )
						continue;	// next part
					if( t_part->min_x - part->max_x > clr )
						continue;	// next part
					if( part->min_y - t_part->max_y > clr )
						continue;	// next part
					if( t_part->min_y - part->max_y > clr )
						continue;	// next part

					// no clearance, we need to test pins in these parts
					// iterate through pins in t_part
					for( int t_ip=0; t_ip<t_s->GetNumPins(); t_ip++ )
					{
						padstack * t_ps = &t_s->m_padstack[t_ip];
						part_pin * t_pin = &t_part->pin[t_ip];
						drc_pin * t_drp = &t_pin->drc;
						id id1 = part->m_id;
						id1.st = ID_PAD;
						id1.i = t_ip;

						// iterate through pins in part
						for( int ip=0; ip<s->GetNumPins(); ip++ )
						{
							padstack * ps = &s->m_padstack[ip];
							part_pin * pin = &part->pin[ip];
							drc_pin * drp = &pin->drc;
							id id2 = part->m_id;
							id2.st = ID_PAD;
							id2.i = ip;

							// test for hole-hole violation
							if( drp->hole_size && t_drp->hole_size )
							{
								// test for hole-to-hole violation
								int dist = Distance( pin->x, pin->y, t_pin->x, t_pin->y );
								int h_h = max( 0, dist - (ps->hole_size + t_ps->hole_size)/2 );
								if( h_h < dr->hole_hole )
								{
									// PADHOLE_PADHOLE
									::MakeCStringFromDimension( &d_str, h_h, units, TRUE, TRUE, TRUE, 1 );
									::MakeCStringFromDimension( &x_str, pin->x, units, FALSE, TRUE, TRUE, 1 );
									::MakeCStringFromDimension( &y_str, pin->y, units, FALSE, TRUE, TRUE, 1 );
									str.Format( "%ld: %s.%s pad hole to %s.%s pad hole = %s, x=%s, y=%s\r\n",  
										nerrors+1, part->ref_des, s->m_padstack[ip].name,
										t_part->ref_des, t_s->m_padstack[t_ip].name,
										d_str, x_str, y_str );
									DRError * dre = drelist->Add( nerrors, DRError::PADHOLE_PADHOLE, &str,
										&t_part->ref_des, &part->ref_des, id1, id2, 
										pin->x, pin->y, t_pin->x, t_pin->y, 0, 0 );
									if( dre )
									{
										nerrors++;
										if( log )
											log->AddLine( str );
									}
								}
							}

							// see if pads on same layers
							if( !(drp->layers & t_drp->layers) )
							{
								// no, see if either has a hole
								if( !drp->hole_size && !t_drp->hole_size )
								{
									// no, go to next pin
									continue;
								}
							}

							// see if padstacks might intersect
							if( drp->min_x - t_drp->max_x > clr )
								continue;	// no, next pin
							if( t_drp->min_x - drp->max_x > clr )
								continue;	// no, next pin
							if( drp->min_y - t_drp->max_y > clr )
								continue;	// no, next pin
							if( t_drp->min_y - drp->max_y > clr )
								continue;	// no, next pin

							// OK, pads might be too close
							// check for pad clearance violations on each layer
							for( int il=0; il<copper_layers; il++ )
							{
								int layer = il + LAY_TOP_COPPER;
								CString lay_str = layer_str[layer];
								int t_pad_x, t_pad_y, t_pad_w, t_pad_l, t_pad_r;
								int t_pad_type, t_pad_hole, t_pad_connect, t_pad_angle;
								cnet * t_pad_net;

								// test for pad-pad violation
								BOOL t_bPad = GetPadDrawInfo( t_part, t_ip, layer, 0, 0, 0, 0,
									&t_pad_type, &t_pad_x, &t_pad_y, &t_pad_w, &t_pad_l, &t_pad_r, 
									&t_pad_hole, &t_pad_angle,
									&t_pad_net, &t_pad_connect );
								if( t_bPad )
								{
									// get pad info for pin
									int pad_x, pad_y, pad_w, pad_l, pad_r;
									int pad_type, pad_hole, pad_connect, pad_angle;
									cnet * pad_net;
									BOOL bPad = GetPadDrawInfo( part, ip, layer, 0, 0, 0, 0, 
										&pad_type, &pad_x, &pad_y, &pad_w, &pad_l, &pad_r, 
										&pad_hole, &pad_angle, &pad_net, &pad_connect );
									if( bPad )
									{
										if( pad_hole )
										{
											// test for pad-padhole violation
											int dist = GetClearanceBetweenPads( t_pad_type, t_pad_x, t_pad_y, 
												t_pad_w, t_pad_l, t_pad_r, t_pad_angle,
												PAD_ROUND, pad_x, pad_y, pad_hole, 0, 0, 0 );
											if( dist < dr->hole_copper )
											{
												// PAD_PADHOLE 
												::MakeCStringFromDimension( &d_str, dist, units, TRUE, TRUE, TRUE, 1 );
												::MakeCStringFromDimension( &x_str, pad_x, units, FALSE, TRUE, TRUE, 1 );
												::MakeCStringFromDimension( &y_str, pad_y, units, FALSE, TRUE, TRUE, 1 );
												str.Format( "%ld: %s.%s pad hole to %s.%s pad = %s, x=%s, y=%s\r\n",  
													nerrors+1, part->ref_des, s->m_padstack[ip].name,
													t_part->ref_des, t_s->m_padstack[t_ip].name,
													d_str, x_str, y_str );
												DRError * dre = drelist->Add( nerrors, DRError::PAD_PADHOLE, &str, 
													&t_part->ref_des, &part->ref_des, id1, id2, 
													pad_x, pad_y, t_pad_x, t_pad_y, 0, layer );
												if( dre )
												{
													nerrors++;
													if( log )
														log->AddLine( str );
												}
												break;		// skip any more layers, go to next pin
											}
										}
										// test for pad-pad violation
										int dist = GetClearanceBetweenPads( t_pad_type, t_pad_x, t_pad_y, 
											t_pad_w, t_pad_l, t_pad_r, t_pad_angle,
											pad_type, pad_x, pad_y, pad_w, pad_l, pad_r, pad_angle );
										if( dist < dr->pad_pad )
										{
											// PAD_PAD 
											::MakeCStringFromDimension( &d_str, dist, units, TRUE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &x_str, pad_x, units, FALSE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &y_str, pad_y, units, FALSE, TRUE, TRUE, 1 );
											str.Format( "%ld: %s.%s pad to %s.%s pad = %s, x=%s, y=%s\r\n",  
												nerrors+1, part->ref_des, s->m_padstack[ip].name,
												t_part->ref_des, t_s->m_padstack[t_ip].name,
												d_str, x_str, y_str );
											DRError * dre = drelist->Add( nerrors, DRError::PAD_PAD, &str, 
												&t_part->ref_des, &part->ref_des, id1, id2, 
												pad_x, pad_y, t_pad_x, t_pad_y, 0, layer );
											if( dre )
											{
												nerrors++;
												if( log )
													log->AddLine( str );
											}
											break;		// skip any more layers, go to next pin
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// iterate through all nets
	str.Format( "\r\nChecking nets and parts:\r\n" );
	if( log )
		log->AddLine( str );
	POSITION pos;
	void * ptr;
	CString name;
	for( pos = m_nlist->m_map.GetStartPosition(); pos != NULL; )
	{
		m_nlist->m_map.GetNextAssoc( pos, name, ptr );
		cnet * net = (cnet*)ptr;
		// iterate through copper areas
		for( int ia=0; ia<net->nareas; ia++ )
		{
			carea * a = &net->area[ia];
			// iterate through contours
			for( int icont=0; icont<a->poly->GetNumContours(); icont++ )
			{
				// iterate through corners and sides
				int istart = a->poly->GetContourStart(icont);
				int iend = a->poly->GetContourEnd(icont);
				for( int ic=istart; ic<=iend; ic++ )
				{
					id id_a = net->id;
					id_a.st = ID_AREA;
					id_a.i = ia;
					id_a.sst = ID_SIDE;
					id_a.ii = ic;
					int x1 = a->poly->GetX(ic);
					int y1 = a->poly->GetY(ic);
					int x2, y2;
					if( ic < iend )
					{
						x2 = a->poly->GetX(ic+1);
						y2 = a->poly->GetY(ic+1);
					}
					else
					{
						x2 = a->poly->GetX(istart);
						y2 = a->poly->GetY(istart);
					}
					int style = a->poly->GetSideStyle(ic);

					// test clearance to board edge
					// iterate through board outlines
					for( int ib=0; ib<board_outline->GetSize(); ib++ )
					{
						CPolyLine * b = &(*board_outline)[ib];
						// iterate through sides
						for( int ibc=0; ibc<b->GetNumCorners(); ibc++ )
						{
							int bx1 = b->GetX(ibc);
							int by1 = b->GetY(ibc);
							int bx2 = b->GetX(0);
							int by2 = b->GetY(0);
							if( ibc != b->GetNumCorners()-1 )
							{
								bx2 = b->GetX(ibc+1);
								by2 = b->GetY(ibc+1);
							}
							int bstyle = b->GetSideStyle(ibc);
							int x, y;
							int d = ::GetClearanceBetweenSegments( bx1, by1, bx2, by2, bstyle, 0,
								x1, y1, x2, y2, style, 0, dr->board_edge_copper, &x, &y );
							if( d < dr->board_edge_copper )
							{
								// BOARDEDGE_COPPERAREA error
								::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
								::MakeCStringFromDimension( &x_str, x, units, FALSE, TRUE, TRUE, 1 );
								::MakeCStringFromDimension( &y_str, y, units, FALSE, TRUE, TRUE, 1 );
								str.Format( "%ld: \"%s\" copper area to board edge = %s, x=%s, y=%s\r\n",  
									nerrors+1, net->name, d_str, x_str, y_str );
								DRError * dre = drelist->Add( nerrors, DRError::BOARDEDGE_COPPERAREA, &str,
									&net->name, NULL, id_a, id_a, x, y, 0, 0, 0, 0 );
								if( dre )
								{
									nerrors++;
									if( log )
										log->AddLine( str );
								}
							}
						}
					}
				}
			}
		}
		// iterate through all connections
		for( int ic=0; ic<net->nconnects; ic++ )
		{
			cconnect * c = &net->connect[ic];
			// get DRC info for this connection
			// iterate through all segments and vertices
			c->min_x = INT_MAX;		// bounding box for connection
			c->max_x = INT_MIN;
			c->min_y = INT_MAX;
			c->max_y = INT_MIN;
			c->vias_present = FALSE;
			c->seg_layers = 0;
			int max_trace_w = 0;	// maximum trace width for connection
			for( int is=0; is<c->nsegs; is++ )
			{
				id id_seg = net->id;
				id_seg.st = ID_CONNECT;
				id_seg.i = ic;
				id_seg.sst = ID_SEG;
				id_seg.ii = is;
				int x1 = c->vtx[is].x;
				int y1 = c->vtx[is].y;
				int x2 = c->vtx[is+1].x;
				int y2 = c->vtx[is+1].y;
				int w = c->seg[is].width;
				int layer = c->seg[is].layer;
				if( c->seg[is].layer >= LAY_TOP_COPPER )
				{
					int layer_bit = c->seg[is].layer - LAY_TOP_COPPER;
					c->seg_layers |= 1<<layer_bit;
				}
				// add segment to bounding box
				int seg_min_x = min( x1, x2 );
				int seg_min_y = min( y1, y2 );
				int seg_max_x = max( x1, x2 );
				int seg_max_y = max( y1, y2 );
				c->min_x = min( c->min_x, seg_min_x - w/2 );
				c->max_x = max( c->max_x, seg_max_x + w/2 );
				c->min_y = min( c->min_y, seg_min_y - w/2 );
				c->max_y = max( c->max_y, seg_max_y + w/2 );
				// test trace width
				if( w > 0 && w < dr->trace_width )
				{
					// TRACE_WIDTH error
					int x = (x1+x2)/2;
					int y = (y1+y2)/2;
					::MakeCStringFromDimension( &d_str, w, units, TRUE, TRUE, TRUE, 1 );
					::MakeCStringFromDimension( &x_str, x, units, FALSE, TRUE, TRUE, 1 );
					::MakeCStringFromDimension( &y_str, y, units, FALSE, TRUE, TRUE, 1 );
					str.Format( "%ld: \"%s\" trace width = %s, x=%s, y=%s\r\n", 
						nerrors+1, net->name, d_str, x_str, y_str );
					DRError * dre = drelist->Add( nerrors, DRError::TRACE_WIDTH, &str, 
						&net->name, NULL, id_seg, id_seg, x, y, 0, 0, 0, layer );
					if( dre )
					{
						nerrors++;
						if( log )
							log->AddLine( str );
					}
				}
				// test clearance to board edge
				if( w > 0 )
				{
					for( int ib=0; ib<board_outline->GetSize(); ib++ )
					{
						CPolyLine * b = &(*board_outline)[ib];
						for( int ibc=0; ibc<b->GetNumCorners(); ibc++ )
						{
							int bx1 = b->GetX(ibc);
							int by1 = b->GetY(ibc);
							int bx2 = b->GetX(0);
							int by2 = b->GetY(0);
							if( ibc != b->GetNumCorners()-1 )
							{
								bx2 = b->GetX(ibc+1);
								by2 = b->GetY(ibc+1);
							}
							int bstyle = b->GetSideStyle(ibc);
							int x, y;
							int d = ::GetClearanceBetweenSegments( bx1, by1, bx2, by2, bstyle, 0,
								x1, y1, x2, y2, CPolyLine::STRAIGHT, w, dr->board_edge_copper, &x, &y );
							if( d < dr->board_edge_copper )
							{
								// BOARDEDGE_TRACE error
								::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
								::MakeCStringFromDimension( &x_str, x, units, FALSE, TRUE, TRUE, 1 );
								::MakeCStringFromDimension( &y_str, y, units, FALSE, TRUE, TRUE, 1 );
								str.Format( "%ld: \"%s\" trace to board edge = %s, x=%s, y=%s\r\n",  
									nerrors+1, net->name, d_str, x_str, y_str );
								DRError * dre = drelist->Add( nerrors, DRError::BOARDEDGE_TRACE, &str,
									&net->name, NULL, id_seg, id_seg, x, y, 0, 0, 0, layer );
								if( dre )
								{
									nerrors++;
									if( log )
										log->AddLine( str );
								}
							}
						}
					}
				}
			}
			for( int iv=0; iv<c->nsegs+1; iv++ )
			{
				cvertex * vtx = &c->vtx[iv];
				if( vtx->via_w )
				{
					// via present
					id id_via = net->id;
					id_via.st = ID_CONNECT;
					id_via.i = ic;
					id_via.sst = ID_VIA;
					id_via.ii = iv;
					c->vias_present = TRUE;
					int min_via_w = INT_MAX;	// minimum via pad diameter
					int max_via_w = INT_MIN;	// maximum via_pad diameter
					for( int il=0; il<copper_layers; il++ )
					{
						int layer = il + LAY_TOP_COPPER;
						int test;
						int pad_w;
						int hole_w;
						m_nlist->GetViaPadInfo( net, ic, iv, layer, 
							&pad_w, &hole_w, &test );
						if( pad_w > 0 )
							min_via_w = min( min_via_w, pad_w );
						max_via_w = max( max_via_w, pad_w );
					}
					if( max_via_w == 0 )
						ASSERT(0);
					c->min_x = min( c->min_x, vtx->x - max_via_w/2 );
					c->max_x = max( c->max_x, vtx->x + max_via_w/2 );
					c->min_y = min( c->min_y, vtx->y - max_via_w/2 );
					c->max_y = max( c->max_y, vtx->y + max_via_w/2 );
					int d = (min_via_w - vtx->via_hole_w)/2;
					if( d < dr->annular_ring_vias )
					{
						// RING_VIA
						::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
						::MakeCStringFromDimension( &x_str, vtx->x, units, FALSE, TRUE, TRUE, 1 );
						::MakeCStringFromDimension( &y_str, vtx->y, units, FALSE, TRUE, TRUE, 1 );
						str.Format( "%ld: \"%s\" via annular ring = %s, x=%s, y=%s\r\n", 
							nerrors+1, net->name, d_str, x_str, y_str );
						DRError * dre = drelist->Add( nerrors, DRError::RING_VIA, &str, 
							&net->name, NULL, id_via, id_via, vtx->x, vtx->y, 0, 0, vtx->via_w+20*NM_PER_MIL, 0 );
						if( dre )
						{
							nerrors++;
							if( log )
								log->AddLine( str );
						}
					}
					// test clearance to board edge
					for( int ib=0; ib<board_outline->GetSize(); ib++ )
					{
						CPolyLine * b = &(*board_outline)[ib];
						for( int ibc=0; ibc<b->GetNumCorners(); ibc++ )
						{
							int bx1 = b->GetX(ibc);
							int by1 = b->GetY(ibc);
							int bx2 = b->GetX(0);
							int by2 = b->GetY(0);
							if( ibc != b->GetNumCorners()-1 )
							{
								bx2 = b->GetX(ibc+1);
								by2 = b->GetY(ibc+1);
							}
							//** for now, only works for straight board edge segments
							if( b->GetSideStyle(ibc) == CPolyLine::STRAIGHT )
							{
								int d = ::GetClearanceBetweenSegmentAndPad( bx1, by1, bx2, by2, 0,
									PAD_ROUND, vtx->x, vtx->y, max_via_w, 0, 0, 0 );
								if( d < dr->board_edge_copper )
								{
									// BOARDEDGE_VIA error
									::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
									::MakeCStringFromDimension( &x_str, vtx->x, units, FALSE, TRUE, TRUE, 1 );
									::MakeCStringFromDimension( &y_str, vtx->y, units, FALSE, TRUE, TRUE, 1 );
									str.Format( "%ld: \"%s\" via to board edge = %s, x=%s, y=%s\r\n",  
										nerrors+1, net->name, d_str, x_str, y_str );
									DRError * dre = drelist->Add( nerrors, DRError::BOARDEDGE_VIA, &str,
										&net->name, NULL, id_via, id_via, vtx->x, vtx->y, 0, 0, vtx->via_w+20*NM_PER_MIL, 0 );
									if( dre )
									{
										nerrors++;
										if( log )
											log->AddLine( str );
									}
								}
								int dh = ::GetClearanceBetweenSegmentAndPad( bx1, by1, bx2, by2, 0,
									PAD_ROUND, vtx->x, vtx->y, vtx->via_hole_w, 0, 0, 0 );
								if( dh < dr->board_edge_hole )
								{
									// BOARDEDGE_VIAHOLE error
									::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
									::MakeCStringFromDimension( &x_str, vtx->x, units, FALSE, TRUE, TRUE, 1 );
									::MakeCStringFromDimension( &y_str, vtx->y, units, FALSE, TRUE, TRUE, 1 );
									str.Format( "%ld: \"%s\" via hole to board edge = %s, x=%s, y=%s\r\n",  
										nerrors+1, net->name, d_str, x_str, y_str );
									DRError * dre = drelist->Add( nerrors, DRError::BOARDEDGE_VIAHOLE, &str,
										&net->name, NULL, id_via, id_via, vtx->x, vtx->y, 0, 0, vtx->via_w+20*NM_PER_MIL, 0 );
									if( dre )
									{
										nerrors++;
										if( log )
											log->AddLine( str );
									}
								}
							}
						}
					}
				}
			}
			// iterate through all parts
			cpart * part = GetFirstPart();
			for( ; part; part = GetNextPart( part ) )
			{
				CShape * s = part->shape;

				// if not on same layers, can't conflict
				if( !part->hole_flag && !c->vias_present && !(part->layers & c->seg_layers) )
					continue;	// next part

				// if bounding boxes don't overlap, can't conflict
				if( part->min_x - c->max_x > dr->pad_trace )
					continue;	// next part
				if( c->min_x - part->max_x > dr->pad_trace )
					continue;	// next part
				if( part->min_y - c->max_y > dr->pad_trace )
					continue;	// next part
				if( c->min_y - part->max_y > dr->pad_trace )
					continue;	// next part

				// OK, now we have to test each pad
				for( int ip=0; ip<part->shape->GetNumPins(); ip++ )
				{
					padstack * ps = &s->m_padstack[ip];
					part_pin * pin = &part->pin[ip];
					drc_pin * drp = &pin->drc;
					id id_pad = part->m_id;
					id_pad.st = ID_PAD;
					id_pad.i = ip;

					// if pin and connection bounds are separated enough, skip pin
					if( drp->min_x - c->max_x > dr->pad_trace )
						continue;	// no, next pin
					if( c->min_x - drp->max_x > dr->pad_trace )
						continue;	// no, next pin
					if( drp->min_y - c->max_y > dr->pad_trace )
						continue;	// no, next pin
					if( c->min_y - drp->max_y > dr->pad_trace )
						continue;	// no, next pin

					// possible clearance violation, now test each segment and via on each layer
					int pad_x, pad_y, pad_w, pad_l, pad_r;
					int pad_type, pad_hole, pad_connect, pad_angle;
					cnet * pad_net;
					BOOL bPad;
					BOOL pin_info_valid = FALSE;
					int pin_info_layer = 0;

					for( int is=0; is<c->nsegs; is++ )
					{
						// get next segment
						cseg * s = &(net->connect[ic].seg[is]);
						cvertex * pre_vtx = &(net->connect[ic].vtx[is]);
						cvertex * post_vtx = &(net->connect[ic].vtx[is+1]);
						int w = s->width;
						int xi = pre_vtx->x;
						int yi = pre_vtx->y;
						int xf = post_vtx->x;
						int yf = post_vtx->y;
						int min_x = min( xi, xf ) - w/2;
						int max_x = max( xi, xf ) + w/2;
						int min_y = min( yi, yf ) - w/2;
						int max_y = max( yi, yf ) + w/2;
						// ids
						id id_seg = net->id;
						id_seg.st = ID_CONNECT;
						id_seg.i = ic;
						id_seg.sst = ID_SEG;
						id_seg.ii = is;
						id id_via = net->id;
						id_via.st = ID_CONNECT;
						id_via.i = ic;
						id_via.sst = ID_VIA;
						id_via.ii = is+1;

						// check all layers
						for( int il=0; il<copper_layers; il++ )
						{
							int layer = il + LAY_TOP_COPPER;
							int layer_bit = 1<<il;

							if( s->layer == layer )
							{
								// check segment clearances
								cnet * pin_net = part->pin[ip].net;
								if( drp->hole_size && net != pin_net )
								{
									// pad has hole, check segment to pad_hole clearance
									if( !(pin_info_valid && layer == pin_info_layer) )
									{
										bPad = GetPadDrawInfo( part, ip, layer, 0, 0, 0, 0,
											&pad_type, &pad_x, &pad_y, &pad_w, &pad_l, &pad_r, 
											&pad_hole, &pad_angle, &pad_net, &pad_connect );
										pin_info_valid = TRUE;
										pin_info_layer = layer;
									}
									int d = GetClearanceBetweenSegmentAndPad( xi, yi, xf, yf, w,
										PAD_ROUND, pad_x, pad_y, pad_hole, 0, 0, 0 );
									if( d < dr->hole_copper ) 
									{
										// SEG_PADHOLE
										::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
										::MakeCStringFromDimension( &x_str, pad_x, units, FALSE, TRUE, TRUE, 1 );
										::MakeCStringFromDimension( &y_str, pad_y, units, FALSE, TRUE, TRUE, 1 );
										str.Format( "%ld: \"%s\" trace to %s.%s pad hole = %s, x=%s, y=%s\r\n", 
											nerrors+1, net->name, part->ref_des, ps->name,
											d_str, x_str, y_str );
										DRError * dre = drelist->Add( nerrors, DRError::SEG_PAD, &str, 
											&net->name, &part->ref_des, id_seg, id_pad, pad_x, pad_y, pad_x, pad_y, 
											max(pad_w,pad_l)+20*NM_PER_MIL, layer );
										if( dre )
										{
											nerrors++;
											if( log )
												log->AddLine( str );
										}
									}
								}
								if( layer_bit & drp->layers )
								{
									// pad is on this layer
									// get pad info for pin if necessary
									if( !(pin_info_valid && layer == pin_info_layer) )
									{
										bPad = GetPadDrawInfo( part, ip, layer, 0, 0, 0, 0, 
											&pad_type, &pad_x, &pad_y, &pad_w, &pad_l, &pad_r,
											&pad_hole, &pad_angle, &pad_net, &pad_connect );
										pin_info_valid = TRUE;
										pin_info_layer = layer;
									}
									if( bPad && pad_type != PAD_NONE && net != pad_net )
									{
										// check segment to pad clearance
										int d = GetClearanceBetweenSegmentAndPad( xi, yi, xf, yf, w,
											pad_type, pad_x, pad_y, pad_w, pad_l, pad_r, pad_angle );
										if( d < dr->pad_trace ) 
										{
											// SEG_PAD
											::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &x_str, pad_x, units, FALSE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &y_str, pad_y, units, FALSE, TRUE, TRUE, 1 );
											str.Format( "%ld: \"%s\" trace to %s.%s pad = %s, x=%s, y=%s\r\n", 
												nerrors+1, net->name, part->ref_des, ps->name,
												d_str, x_str, y_str );
											DRError * dre = drelist->Add( nerrors, DRError::SEG_PAD, &str, 
												&net->name, &part->ref_des, id_seg, id_pad, pad_x, pad_y, pad_x, pad_y, 
												max(pad_w,pad_l)+20*NM_PER_MIL, layer );
											if( dre )
											{
												nerrors++;
												if( log )
													log->AddLine( str );
											}
										}
									}
								}
							}
							// get next via
							if( post_vtx->via_w )
							{
								// via exists
								int test;
								int via_w;
								int via_hole_w;
								m_nlist->GetViaPadInfo( net, ic, is+1, layer, 
									&via_w, &via_hole_w, &test );
								int w = 0;
								if( via_w )
								{
									// check via_pad to pin_pad clearance
									if( !(pin_info_valid && layer == pin_info_layer) )
									{
										bPad = GetPadDrawInfo( part, ip, layer, 0, 0, 0, 0, 
											&pad_type, &pad_x, &pad_y, &pad_w, &pad_l, &pad_r, 
											&pad_hole, &pad_angle, &pad_net, &pad_connect );
										pin_info_valid = TRUE;
										pin_info_layer = layer;
									}
									if( bPad && pad_type != PAD_NONE && pad_net != net )
									{
										int d = GetClearanceBetweenPads( PAD_ROUND, xf, yf, via_w, 0, 0, 0,
											pad_type, pad_x, pad_y, pad_w, pad_l, pad_r, pad_angle );
										if( d < dr->pad_trace )
										{
											// VIA_PAD
											::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &x_str, pad_x, units, FALSE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &y_str, pad_y, units, FALSE, TRUE, TRUE, 1 );
											str.Format( "%ld: \"%s\" via pad to %s.%s pad = %s, x=%s, y=%s\r\n", 
												nerrors+1, net->name, part->ref_des, ps->name,
												d_str, x_str, y_str );
											DRError * dre = drelist->Add( nerrors, DRError::VIA_PAD, &str, 
												&net->name, &part->ref_des, id_via, id_pad, xf, yf, pad_x, pad_y, 0, layer );
											if( dre )
											{
												nerrors++;
												if( log )
													log->AddLine( str );
											}
											break;  // skip more layers
										}
									}
									if( drp->hole_size && pad_net != net )
									{
										// pin has a hole, check via_pad to pin_hole clearance
										int d = Distance( xf, yf, pin->x, pin->y );
										d = max( 0, d - drp->hole_size/2 - via_w/2 );
										if( d < dr->hole_copper )
										{
											// VIA_PADHOLE
											::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &x_str, pad_x, units, FALSE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &y_str, pad_y, units, FALSE, TRUE, TRUE, 1 );
											str.Format( "%ld: \"%s\" via pad to %s.%s pad hole = %s, x=%s, y=%s\r\n", 
												nerrors+1, net->name, part->ref_des, ps->name,
												d_str, x_str, y_str );
											DRError * dre = drelist->Add( nerrors, DRError::VIA_PAD, &str, 
												&net->name, &part->ref_des, id_via, id_pad, xf, yf, pad_x, pad_y, 0, layer );
											if( dre )
											{
												nerrors++;
												if( log )
													log->AddLine( str );
											}
											break;  // skip more layers
										}
									}
								}
								if( !(pin_info_valid && layer == pin_info_layer) )
								{
									bPad = GetPadDrawInfo( part, ip, layer, 0, 0, 0, 0,
										&pad_type, &pad_x, &pad_y, &pad_w, &pad_l, &pad_r,
										&pad_hole, &pad_angle, &pad_net, &pad_connect );
									pin_info_valid = TRUE;
									pin_info_layer = layer;
								}
								if( bPad && pad_type != PAD_NONE && pad_net != net )
								{
									// check via_hole to pin_pad clearance
									int d = GetClearanceBetweenPads( PAD_ROUND, xf, yf, post_vtx->via_hole_w, 0, 0, 0,
										pad_type, pad_x, pad_y, pad_w, pad_l, pad_r, pad_angle );
									if( d < dr->hole_copper )
									{
										// VIAHOLE_PAD
										::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
										::MakeCStringFromDimension( &x_str, pad_x, units, FALSE, TRUE, TRUE, 1 );
										::MakeCStringFromDimension( &y_str, pad_y, units, FALSE, TRUE, TRUE, 1 );
										str.Format( "%ld: \"%s\" via hole to %s.%s pad = %s, x=%s, y=%s\r\n", 
											nerrors+1, net->name, part->ref_des, ps->name,
											d_str, x_str, y_str );
										DRError * dre = drelist->Add( nerrors, DRError::VIA_PAD, &str, 
											&net->name, &part->ref_des, id_via, id_pad, xf, yf, pad_x, pad_y, 0, layer );
										if( dre )
										{
											nerrors++;
											if( log )
												log->AddLine( str );
										}
										break;  // skip more layers
									}
								}
								if( drp->hole_size && layer == LAY_TOP_COPPER )
								{
									// pin has a hole, check via_hole to pin_hole clearance
									int d = Distance( xf, yf, pin->x, pin->y );
									d = max( 0, d - drp->hole_size/2 - post_vtx->via_hole_w/2 );
									if( d < dr->hole_hole )
									{
										// VIAHOLE_PADHOLE
										::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
										::MakeCStringFromDimension( &x_str, pad_x, units, FALSE, TRUE, TRUE, 1 );
										::MakeCStringFromDimension( &y_str, pad_y, units, FALSE, TRUE, TRUE, 1 );
										str.Format( "%ld: \"%s\" via hole to %s.%s pad hole = %s, x=%s, y=%s\r\n", 
											nerrors+1, net->name, part->ref_des, ps->name,
											d_str, x_str, y_str );
										DRError * dre = drelist->Add( nerrors, DRError::VIA_PAD, &str, 
											&net->name, &part->ref_des, id_via, id_pad, xf, yf, pad_x, pad_y, 0, layer );
										if( dre )
										{
											nerrors++;
											if( log )
												log->AddLine( str );
										}
										break;  // skip more layers
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// now check nets against other nets
	str.Format( "\r\nChecking nets:\r\n" );
	if( log )
		log->AddLine( str );
	// get max clearance
	int cl = max( dr->hole_copper, dr->hole_hole );
	cl = max( cl, dr->trace_trace );
	// iterate through all nets
	for( pos = m_nlist->m_map.GetStartPosition(); pos != NULL; )
	{
		m_nlist->m_map.GetNextAssoc( pos, name, ptr );
		cnet * net = (cnet*)ptr;
		// iterate through all connections
		for( int ic=0; ic<net->nconnects; ic++ )
		{
			cconnect * c = &net->connect[ic];

			// iterate through all nets again
			POSITION pos2 = pos;
			void * ptr2;
			CString name2;
			while( pos2 != NULL )
			{
				m_nlist->m_map.GetNextAssoc( pos2, name2, ptr2 );
				cnet * net2 = (cnet*)ptr2;
				// iterate through all connections
				for( int ic2=0; ic2<net2->nconnects; ic2++ )
				{
					cconnect * c2 = &net2->connect[ic2];
					// look for possible clearance violations between c and c2
					if( c->min_x - c2->max_x > cl )
						continue;	// no, next connection
					if( c->min_y - c2->max_y > cl )
						continue;	// no, next connection
					if( c2->min_x - c->max_x > cl )
						continue;	// no, next connection
					if( c2->min_y - c->max_y > cl )
						continue;	// no, next connection

					// now we have to test all segments and vias in c
					for( int is=0; is<c->nsegs; is++ )
					{
						// get next segment and via
						cseg * s = &c->seg[is];
						cvertex * pre_vtx = &c->vtx[is];
						cvertex * post_vtx = &c->vtx[is+1];
						int seg_w = s->width;
						int vw = post_vtx->via_w;
						int max_w = max( seg_w, vw );
						int xi = pre_vtx->x;
						int yi = pre_vtx->y;
						int xf = post_vtx->x;
						int yf = post_vtx->y;
						// get bounding rect for segment and vias
						int min_x = min( xi, xf ) - max_w/2;
						int max_x = max( xi, xf ) + max_w/2;
						int min_y = min( yi, yf ) - max_w/2;
						int max_y = max( yi, yf ) + max_w/2;
						// ids
						id id_seg1( ID_NET, ID_CONNECT, ic, ID_SEG, is );
						id id_via1( ID_NET, ID_CONNECT, ic, ID_VIA, is+1 );

						// iterate through all segments and vias in c2
						for( int is2=0; is2<c2->nsegs; is2++ )
						{
							// get next segment and via
							cseg * s2 = &c2->seg[is2];
							cvertex * pre_vtx2 = &c2->vtx[is2];
							cvertex * post_vtx2 = &c2->vtx[is2+1];
							int seg_w2 = s2->width;
							int vw2 = post_vtx2->via_w;
							int max_w2 = max( seg_w2, vw2 );
							int xi2 = pre_vtx2->x;
							int yi2 = pre_vtx2->y;
							int xf2 = post_vtx2->x;
							int yf2 = post_vtx2->y;
							// get bounding rect for this segment and attached vias
							int min_x2 = min( xi2, xf2 ) - max_w2/2;
							int max_x2 = max( xi2, xf2 ) + max_w2/2;
							int min_y2 = min( yi2, yf2 ) - max_w2/2;
							int max_y2 = max( yi2, yf2 ) + max_w2/2;
							// ids
							id id_seg2( ID_NET, ID_CONNECT, ic2, ID_SEG, is2 );
							id id_via2( ID_NET, ID_CONNECT, ic2, ID_VIA, is2+1 );

							// see if segment bounding rects are too close
							if( min_x - max_x2 > cl )
								continue;	// no, next segment
							if( min_y - max_y2 > cl )
								continue;
							if( min_x2 - max_x > cl )
								continue;
							if( min_y2 - max_y > cl )
								continue;

							// check if segments on same layer
							if( s->layer == s2->layer && s->layer >= LAY_TOP_COPPER ) 
							{
								// yes, test clearances between segments
								int xx, yy; 
								int d = ::GetClearanceBetweenSegments( xi, yi, xf, yf, CPolyLine::STRAIGHT, seg_w, 
									xi2, yi2, xf2, yf2, CPolyLine::STRAIGHT, seg_w2, dr->trace_trace, &xx, &yy );
								if( d < dr->trace_trace )
								{
									// SEG_SEG
									::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
									::MakeCStringFromDimension( &x_str, xx, units, FALSE, TRUE, TRUE, 1 );
									::MakeCStringFromDimension( &y_str, yy, units, FALSE, TRUE, TRUE, 1 );
									str.Format( "%ld: \"%s\" trace to \"%s\" trace = %s, x=%s, y=%s\r\n", 
										nerrors+1, net->name, net2->name,
										d_str, x_str, y_str );
									DRError * dre = drelist->Add( nerrors, DRError::SEG_SEG, &str, 
										&net->name, &net2->name, id_seg1, id_seg2, xx, yy, xx, yy, 0, s->layer );
									if( dre )
									{
										nerrors++;
										if( log )
											log->AddLine( str );
									}
								}
							}
							// test clearances between net->segment and net2->via
							int layer = s->layer;
							if( layer >= LAY_TOP_COPPER && post_vtx2->via_w )
							{
								// via exists
								int test = m_nlist->GetViaConnectionStatus( net2, ic2, is2+1, layer );
								int via_w2 = post_vtx2->via_w;	// normal via pad
								if( layer > LAY_BOTTOM_COPPER && test == CNetList::VIA_NO_CONNECT )
								{
									// inner layer and no trace or thermal, so no via pad
									via_w2 = 0;
								}
								else if( layer > LAY_BOTTOM_COPPER && (test & CNetList::VIA_AREA) && !(test & CNetList::VIA_TRACE) )
								{
									// inner layer with small thermal, use annular ring
									via_w2 = post_vtx2->via_hole_w + 2*dr->annular_ring_vias;
								}
								// check clearance
								if( via_w2 )
								{
									// check clearance between segment and via pad
									int d = GetClearanceBetweenSegmentAndPad( xi, yi, xf, yf, seg_w,
										PAD_ROUND, post_vtx2->x, post_vtx2->y, post_vtx2->via_w, 0, 0, 0 );
									if( d < dr->trace_trace )
									{
										// SEG_VIA
										::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
										::MakeCStringFromDimension( &x_str, post_vtx2->x, units, FALSE, TRUE, TRUE, 1 );
										::MakeCStringFromDimension( &y_str, post_vtx2->y, units, FALSE, TRUE, TRUE, 1 );
										str.Format( "%ld: \"%s\" trace to \"%s\" via pad = %s, x=%s, y=%s\r\n", 
											nerrors+1, net->name, net2->name,
											d_str, x_str, y_str );
										DRError * dre = drelist->Add( nerrors, DRError::SEG_VIA, &str, 
											&net->name, &net2->name, id_seg1, id_via2, xf2, yf2, xf2, yf2, 0, s->layer );
										if( dre )
										{
											nerrors++;
											if( log )
												log->AddLine( str );
										}
									}
								}
								// check clearance between segment and via hole
								int d = GetClearanceBetweenSegmentAndPad( xi, yi, xf, yf, seg_w,
									PAD_ROUND, post_vtx2->x, post_vtx2->y, post_vtx2->via_hole_w, 0, 0, 0 );
								if( d < dr->hole_copper )
								{
									// SEG_VIAHOLE
									::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
									::MakeCStringFromDimension( &x_str, post_vtx2->x, units, FALSE, TRUE, TRUE, 1 );
									::MakeCStringFromDimension( &y_str, post_vtx2->y, units, FALSE, TRUE, TRUE, 1 );
									str.Format( "%ld: \"%s\" trace to \"%s\" via hole = %s, x=%s, y=%s\r\n", 
										nerrors+1, net->name, net2->name,
										d_str, x_str, y_str );
									DRError * dre = drelist->Add( nerrors, DRError::SEG_VIAHOLE, &str, 
										&net->name, &net2->name, id_seg1, id_via2, xf2, yf2, xf2, yf2, 0, s->layer );
									if( dre )
									{
										nerrors++;
										if( log )
											log->AddLine( str );
									}
								}
							}
							// test clearances between net2->segment and net->via
							layer = s2->layer;
							if( post_vtx->via_w )
							{
								// via exists
								int test = m_nlist->GetViaConnectionStatus( net, ic, is+1, layer );
								int via_w = post_vtx->via_w;	// normal via pad
								if( layer > LAY_BOTTOM_COPPER && test == CNetList::VIA_NO_CONNECT )
								{
									// inner layer and no trace or thermal, so no via pad
									via_w = 0;
								}
								else if( layer > LAY_BOTTOM_COPPER && (test & CNetList::VIA_AREA) && !(test & CNetList::VIA_TRACE) )
								{
									// inner layer with small thermal, use annular ring
									via_w = post_vtx->via_hole_w + 2*dr->annular_ring_vias;
								}
								// check clearance
								if( via_w )
								{
									// check clearance between net2->segment and net->via_pad
									if( layer >= LAY_TOP_COPPER )
									{
										int d = GetClearanceBetweenSegmentAndPad( xi2, yi2, xf2, yf2, seg_w2,
											PAD_ROUND, post_vtx->x, post_vtx->y, post_vtx->via_w, 0, 0, 0 );
										if( d < dr->trace_trace )
										{
											// SEG_VIA
											::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &x_str, post_vtx->x, units, FALSE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &y_str, post_vtx->y, units, FALSE, TRUE, TRUE, 1 );
											str.Format( "%ld: \"%s\" via pad to \"%s\" trace = %s, x=%s, y=%s\r\n", 
												nerrors+1, net->name, net2->name,
												d_str, x_str, y_str );
											DRError * dre = drelist->Add( nerrors, DRError::SEG_VIA, &str, 
												&net2->name, &net->name, id_seg2, id_via1, xf, yf, xf, yf, 
												post_vtx->via_w+20*NM_PER_MIL, 0 );
											if( dre )
											{
												nerrors++;
												if( log )
													log->AddLine( str );
											}
										}
									}
								}
								// check clearance between net2->segment and net->via_hole
								if( layer >= LAY_TOP_COPPER )
								{
									int d = GetClearanceBetweenSegmentAndPad( xi2, yi2, xf2, yf2, seg_w2,
										PAD_ROUND, post_vtx->x, post_vtx->y, post_vtx->via_hole_w, 0, 0, 0 );
									if( d < dr->hole_copper )
									{
										// SEG_VIAHOLE
										::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
										::MakeCStringFromDimension( &x_str, post_vtx->x, units, FALSE, TRUE, TRUE, 1 );
										::MakeCStringFromDimension( &y_str, post_vtx->y, units, FALSE, TRUE, TRUE, 1 );
										str.Format( "%ld: \"%s\" trace to \"%s\" via hole = %s, x=%s, y=%s\r\n", 
											nerrors+1, net2->name, net->name,
											d_str, x_str, y_str );
										DRError * dre = drelist->Add( nerrors, DRError::SEG_VIAHOLE, &str, 
											&net2->name, &net->name, id_seg2, id_via1, xf, yf, xf, yf, 
											post_vtx->via_w+20*NM_PER_MIL, 0 );
										if( dre )
										{
											nerrors++;
											if( log )
												log->AddLine( str );
										}
									}
								}
								// test clearances between net->via and net2->via
								if( post_vtx->via_w && post_vtx2->via_w )
								{
									for( int layer=LAY_TOP_COPPER; layer<(LAY_TOP_COPPER+copper_layers); layer++ )
									{
										// get size of net->via_pad
										int test = m_nlist->GetViaConnectionStatus( net, ic, is+1, layer );
										int via_w = post_vtx->via_w;	// normal via pad
										if( layer > LAY_BOTTOM_COPPER && test == CNetList::VIA_NO_CONNECT )
										{
											// inner layer and no trace or thermal, so no via pad
											via_w = 0;
										}
										else if( layer > LAY_BOTTOM_COPPER && (test & CNetList::VIA_AREA) && !(test & CNetList::VIA_TRACE) )
										{
											// inner layer with small thermal, use annular ring
											via_w = post_vtx->via_hole_w + 2*dr->annular_ring_vias;
										}
										// get size of net2->via_pad
										test = m_nlist->GetViaConnectionStatus( net2, ic2, is2+1, layer );
										int via_w2 = post_vtx2->via_w;	// normal via pad
										if( layer > LAY_BOTTOM_COPPER && test == CNetList::VIA_NO_CONNECT )
										{
											// inner layer and no trace or thermal, so no via pad
											via_w2 = 0;
										}
										else if( layer > LAY_BOTTOM_COPPER && (test & CNetList::VIA_AREA) && !(test & CNetList::VIA_TRACE) )
										{
											// inner layer with small thermal, use annular ring
											via_w2 = post_vtx2->via_hole_w + 2*dr->annular_ring_vias;
										}
										if( via_w && via_w2 )
										{
											//check net->via_pad to net2->via_pad clearance
											int d = GetClearanceBetweenPads( PAD_ROUND, post_vtx->x, post_vtx->y, post_vtx->via_w, 0, 0, 0, 
												PAD_ROUND, post_vtx2->x, post_vtx2->y, post_vtx2->via_w, 0, 0, 0 );
											if( d < dr->trace_trace )
											{
												// VIA_VIA
												::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
												::MakeCStringFromDimension( &x_str, post_vtx->x, units, FALSE, TRUE, TRUE, 1 );
												::MakeCStringFromDimension( &y_str, post_vtx->y, units, FALSE, TRUE, TRUE, 1 );
												str.Format( "%ld: \"%s\" via pad to \"%s\" via pad = %s, x=%s, y=%s\r\n", 
													nerrors+1, net->name, net2->name,
													d_str, x_str, y_str );
												DRError * dre = drelist->Add( nerrors, DRError::VIA_VIA, &str, 
													&net->name, &net2->name, id_via1, id_via2, xf, yf, xf2, yf2, 0, layer );
												if( dre )
												{
													nerrors++;
													if( log )
														log->AddLine( str );
												}
											}
											// check net->via to net2->via_hole clearance
											d = GetClearanceBetweenPads( PAD_ROUND, post_vtx->x, post_vtx->y, post_vtx->via_w, 0, 0, 0,
												PAD_ROUND, post_vtx2->x, post_vtx2->y, post_vtx2->via_hole_w, 0, 0, 0 );
											if( d < dr->hole_copper )
											{
												// VIA_VIAHOLE
												::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
												::MakeCStringFromDimension( &x_str, post_vtx->x, units, FALSE, TRUE, TRUE, 1 );
												::MakeCStringFromDimension( &y_str, post_vtx->y, units, FALSE, TRUE, TRUE, 1 );
												str.Format( "%ld: \"%s\" via pad to \"%s\" via hole = %s, x=%s, y=%s\r\n", 
													nerrors+1, net->name, net2->name,
													d_str, x_str, y_str );
												DRError * dre = drelist->Add( nerrors, DRError::VIA_VIAHOLE, &str, 
													&net->name, &net2->name, id_via1, id_via2, xf, yf, xf2, yf2, 0, layer );
												if( dre )
												{
													nerrors++;
													if( log )
														log->AddLine( str );
												}
											}
											// check net2->via to net->via_hole clearance
											d = GetClearanceBetweenPads( PAD_ROUND, post_vtx->x, post_vtx->y, post_vtx->via_hole_w, 0, 0, 0,
												PAD_ROUND, post_vtx2->x, post_vtx2->y, post_vtx2->via_w, 0, 0, 0 );
											if( d < dr->hole_copper )
											{
												// VIA_VIAHOLE
												::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
												::MakeCStringFromDimension( &x_str, post_vtx->x, units, FALSE, TRUE, TRUE, 1 );
												::MakeCStringFromDimension( &y_str, post_vtx->y, units, FALSE, TRUE, TRUE, 1 );
												str.Format( "%ld: \"%s\" via pad to \"%s\" via hole = %s, x=%s, y=%s\r\n", 
													nerrors+1, net2->name, net->name,
													d_str, x_str, y_str );
												DRError * dre = drelist->Add( nerrors, DRError::VIA_VIAHOLE, &str, 
													&net2->name, &net->name, id_via2, id_via1, xf, yf, xf2, yf2, 0, layer );
												if( dre )
												{
													nerrors++;
													if( log )
														log->AddLine( str );
												}
											}
										}
									}
									// check net->via_hole to net2->via_hole clearance
									int d = GetClearanceBetweenPads( PAD_ROUND, post_vtx->x, post_vtx->y, post_vtx->via_hole_w, 0, 0, 0,
										PAD_ROUND, post_vtx2->x, post_vtx2->y, post_vtx2->via_hole_w, 0, 0,0  );
									if( d < dr->hole_hole )
									{
										// VIA_VIAHOLE
										::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
										::MakeCStringFromDimension( &x_str, post_vtx->x, units, FALSE, TRUE, TRUE, 1 );
										::MakeCStringFromDimension( &y_str, post_vtx->y, units, FALSE, TRUE, TRUE, 1 );
										str.Format( "%ld: \"%s\" via hole to \"%s\" via hole = %s, x=%s, y=%s\r\n", 
											nerrors+1, net2->name, net->name,
											d_str, x_str, y_str );
										DRError * dre = drelist->Add( nerrors, DRError::VIAHOLE_VIAHOLE, &str, 
											&net->name, &net2->name, id_via1, id_via2, xf, yf, xf2, yf2, 0, 0 );
										if( dre )
										{
											nerrors++;
											if( log )
												log->AddLine( str );
										}
									}
								}
							}
						}
					}
				}
			}
		}
		// now iterate through all areas
		for( int ia=0; ia<net->nareas; ia++ )
		{
			carea * a = &net->area[ia];
			// iterate through all nets again
			POSITION pos2 = pos;
			void * ptr2;
			CString name2;
			while( pos2 != NULL )
			{
				m_nlist->m_map.GetNextAssoc( pos2, name2, ptr2 );
				cnet * net2 = (cnet*)ptr2;
				for( int ia2=0; ia2<net2->nareas; ia2++ )
				{
					carea * a2 = &net2->area[ia2];
					// test for same layer
					if( a->poly->GetLayer() == a2->poly->GetLayer() ) 
					{
						// test for points inside one another
						for( int ic=0; ic<a->poly->GetNumCorners(); ic++ )
						{
							int x = a->poly->GetX(ic);
							int y = a->poly->GetY(ic);
							if( a2->poly->TestPointInside( x, y ) )
							{
								// COPPERAREA_COPPERAREA error
								id id_a = net->id;
								id_a.st = ID_AREA;
								id_a.i = ia;
								id_a.sst = ID_SEL_CORNER;
								id_a.ii = ic;
								str.Format( "%ld: \"%s\" copper area inside \"%s\" inside copper area\r\n",  
									nerrors+1, net->name, net2->name );
								DRError * dre = drelist->Add( nerrors, DRError::COPPERAREA_INSIDE_COPPERAREA, &str,
									&net->name, &net2->name, id_a, id_a, x, y, x, y, 0, 0 );
								if( dre )
								{
									nerrors++;
									if( log )
										log->AddLine( str );
								}
							}
						}
						for( int ic2=0; ic2<a2->poly->GetNumCorners(); ic2++ )
						{
							int x = a2->poly->GetX(ic2);
							int y = a2->poly->GetY(ic2);
							if( a->poly->TestPointInside( x, y ) )
							{
								// COPPERAREA_COPPERAREA error
								id id_a = net2->id;
								id_a.st = ID_AREA;
								id_a.i = ia2;
								id_a.sst = ID_SEL_CORNER;
								id_a.ii = ic2;
								str.Format( "%ld: \"%s\" copper area inside \"%s\" copper area\r\n",  
									nerrors+1, net2->name, net->name );
								DRError * dre = drelist->Add( nerrors, DRError::COPPERAREA_INSIDE_COPPERAREA, &str,
									&net2->name, &net->name, id_a, id_a, x, y, x, y, 0, 0 );
								if( dre )
								{
									nerrors++;
									if( log )
										log->AddLine( str );
								}
							}
						}
						// now test spacing between areas
						for( int icont=0; icont<a->poly->GetNumContours(); icont++ )
						{
							int ic_start = a->poly->GetContourStart( icont );
							int ic_end = a->poly->GetContourEnd( icont );
							for( int ic=ic_start; ic<=ic_end; ic++ ) 
							{
								id id_a = net->id;
								id_a.st = ID_AREA;
								id_a.i = ia;
								id_a.sst = ID_SIDE;
								id_a.ii = ic;
								int ax1 = a->poly->GetX(ic);
								int ay1 = a->poly->GetY(ic);
								int ax2, ay2;
								if( ic == ic_end )
								{
									ax2 = a->poly->GetX(ic_start);
									ay2 = a->poly->GetY(ic_start);
								}
								else
								{
									ax2 = a->poly->GetX(ic+1);
									ay2 = a->poly->GetY(ic+1);
								}
								int astyle = a->poly->GetSideStyle(ic);
								for( int icont2=0; icont2<a2->poly->GetNumContours(); icont2++ )
								{
									int ic_start2 = a2->poly->GetContourStart( icont2 );
									int ic_end2 = a2->poly->GetContourEnd( icont2 );
									for( int ic2=ic_start2; ic2<=ic_end2; ic2++ )
									{
										id id_b = net2->id;
										id_b.st = ID_AREA;
										id_b.i = ia2;
										id_b.sst = ID_SIDE;
										id_b.ii = ic2;
										int bx1 = a2->poly->GetX(ic2);
										int by1 = a2->poly->GetY(ic2);
										int bx2, by2;
										if( ic2 == ic_end2 )
										{
											bx2 = a2->poly->GetX(ic_start2);
											by2 = a2->poly->GetY(ic_start2);
										}
										else
										{
											bx2 = a2->poly->GetX(ic2+1);
											by2 = a2->poly->GetY(ic2+1);
										}
										int bstyle = a2->poly->GetSideStyle(ic2);
										int x, y;
										int d = ::GetClearanceBetweenSegments( bx1, by1, bx2, by2, bstyle, 0,
											ax1, ay1, ax2, ay2, astyle, 0, dr->copper_copper, &x, &y );
										if( d < dr->copper_copper )
										{
											// COPPERAREA_COPPERAREA error
											::MakeCStringFromDimension( &d_str, d, units, TRUE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &x_str, x, units, FALSE, TRUE, TRUE, 1 );
											::MakeCStringFromDimension( &y_str, y, units, FALSE, TRUE, TRUE, 1 );
											str.Format( "%ld: \"%s\" copper area to \"%s\" copper area = %s, x=%s, y=%s\r\n",  
												nerrors+1, net->name, net2->name, d_str, x_str, y_str );
											DRError * dre = drelist->Add( nerrors, DRError::COPPERAREA_COPPERAREA, &str,
												&net->name, &net2->name, id_a, id_b, x, y, x, y, 0, 0 );
											if( dre )
											{
												nerrors++;
												if( log )
													log->AddLine( str );
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	// now check for unrouted connections, if requested
	if( check_unrouted )
	{
		for( pos = m_nlist->m_map.GetStartPosition(); pos != NULL; )
		{
			m_nlist->m_map.GetNextAssoc( pos, name, ptr );
			cnet * net = (cnet*)ptr;
			// iterate through all connections
			// now check connections
			for( int ic=0; ic<net->connect.GetSize(); ic++ )
			{
				// check for unrouted or partially routed connection
				BOOL bUnrouted = FALSE;
				for( int is=0; is<net->connect[ic].nsegs; is++ )
				{
					if( net->connect[ic].seg[is].layer == LAY_RAT_LINE )
					{
						bUnrouted = TRUE;
						break;
					}
				}
				if( bUnrouted )
				{
					// unrouted or partially routed connection
					CString start_pin, end_pin;
					int istart = net->connect[ic].start_pin;
					cpart * start_part = net->pin[istart].part;
					start_pin = net->pin[istart].ref_des + "." + net->pin[istart].pin_name;
					int iend = net->connect[ic].end_pin;
					if( iend == cconnect::NO_END )
					{
						str.Format( "%ld: \"%s\": partially routed stub trace from %s\r\n",
							nerrors+1, net->name, start_pin );
						CPoint pt = GetPinPoint( start_part, net->pin[istart].pin_name );
						id id_a = net->id;
						DRError * dre = drelist->Add( nerrors, DRError::UNROUTED, &str,
							&net->name, NULL, id_a, id_a, pt.x, pt.y, pt.x, pt.y, 0, 0 );
						if( dre )
						{
							nerrors++;
							if( log )
								log->AddLine( str );
						}
					}
					else
					{
						end_pin = net->pin[iend].ref_des + "." + net->pin[iend].pin_name;
						if( net->connect[ic].nsegs > 1 )
						{
							str.Format( "%ld: \"%s\": partially routed connection from %s to %s\r\n",
								nerrors+1, net->name, start_pin, end_pin );
						}
						else
						{
							str.Format( "%ld: \"%s\": unrouted connection from %s to %s\r\n",
								nerrors+1, net->name, start_pin, end_pin );
						}
						CPoint pt = GetPinPoint( start_part, net->pin[istart].pin_name );
						id id_a = net->id;
						DRError * dre = drelist->Add( nerrors, DRError::UNROUTED, &str,
							&net->name, NULL, id_a, id_a, pt.x, pt.y, pt.x, pt.y, 0, 0 );
						if( dre )
						{
							nerrors++;
							if( log )
								log->AddLine( str );
						}
					}
				}
			}
		}
	}
	str = "\r\n***** DONE *****\r\n";
	if( log )
		log->AddLine( str );
}

// check partlist for errors
//
int CPartList::CheckPartlist( CString * logstr )
{
	int nerrors = 0;
	int nwarnings = 0;
	CString str;
	CMapStringToPtr map;
	void * ptr;

	*logstr += "***** Checking Parts *****\r\n";

	// first, check for duplicate parts
	cpart * part = m_start.next;
	while( part->next != 0 )
	{
		CString ref_des = part->ref_des;
		BOOL test = map.Lookup( ref_des, ptr );
		if( test )
		{
			str.Format( "ERROR: Part \"%s\" duplicated\r\n", ref_des );
			str += "    ###   To fix this, delete one instance of the part, then save, close and re-open project\r\n";
			*logstr += str;
			nerrors++;
		}
		else
			map.SetAt( ref_des, NULL );

		// next part
		part = part->next;
	}

	// now check all parts
	part = m_start.next;
	while( part->next != 0 )
	{
		// check this part
		str = "";
		CString * ref_des = &part->ref_des;
		if( !part->shape )
		{
			// no footprint
			str.Format( "Warning: Part \"%s\" has no footprint\r\n",
				*ref_des );
			nwarnings++;
		}
		else
		{
			for( int ip=0; ip<part->pin.GetSize(); ip++ )
			{
				// check this pin
				cnet * net = part->pin[ip].net;
				CString * pin_name = &part->shape->m_padstack[ip].name;
				if( !net )
				{
					// part->pin->net is NULL, pin unconnected
					// this is not an error
					//				str.Format( "%s.%s unconnected\r\n",
					//					*ref_des, *pin_name );
				}
				else
				{
					cnet * netlist_net = m_nlist->GetNetPtrByName( &net->name );
					if( !netlist_net )
					{
						// part->pin->net->name doesn't exist in netlist
						str.Format( "ERROR: Part \"%s\" pin \"%s\" connected to net \"%s\" which doesn't exist in netlist\r\n",
							*ref_des, *pin_name, net->name );
						nerrors++;
					}
					else
					{
						if( net != netlist_net )
						{
							// part->pin->net doesn't match netlist->net
							str.Format( "ERROR: Part \"%s\" pin \"%s\" connected to net \"%s\" which doesn't match netlist\r\n",
								*ref_des, *pin_name, net->name );
							nerrors++;
						}
						else
						{
							// try to find pin in pin list for net
							int net_pin = -1;
							for( int ip=0; ip<net->npins; ip++ )
							{
								if( net->pin[ip].part == part )
								{
									if( net->pin[ip].pin_name == *pin_name )
									{
										net_pin = ip;
										break;
									}
								}
							}
							if( net_pin == -1 )
							{
								// pin not found
								str.Format( "ERROR: Part \"%s\" pin \"%s\" connected to net \"%\" but pin not in net\r\n",
									*ref_des, *pin_name, net->name );
								nerrors++;
							}
							else
							{
								// OK
							}

						}
					}
				}
			}
		}
		*logstr += str;

		// next part
		part = part->next;
	}
	str.Format( "***** %d ERROR(S), %d WARNING(S) *****\r\n", nerrors, nwarnings );
	*logstr += str;

	return nerrors;
}

void CPartList::MoveOrigin( int x_off, int y_off )
{
	cpart * part = GetFirstPart();
	while( part )
	{
		if( part->shape )
		{
			// move this part
			UndrawPart( part );
			part->x += x_off;
			part->y += y_off;
			for( int ip=0; ip<part->pin.GetSize(); ip++ )
			{
				part->pin[ip].x += x_off;
				part->pin[ip].y += y_off;
			}
			DrawPart( part );
		}
		part = GetNextPart(part);
	}
}

BOOL CPartList::CheckForProblemFootprints()
{
	BOOL bHeaders_28mil_holes = FALSE;   
	cpart * part = GetFirstPart();
	while( part )
	{
		if( part->shape)
		{
			if( part->shape->m_name.Right(7) == "HDR-100" 
				&& part->shape->m_padstack[0].hole_size == 28*NM_PER_MIL )
			{
				bHeaders_28mil_holes = TRUE;
			}
		}
		part = GetNextPart( part );
	}
	if( g_bShow_header_28mil_hole_warning && bHeaders_28mil_holes )   
	{
		CDlgMyMessageBox dlg;
		dlg.Initialize( "WARNING: You are loading footprint(s) for through-hole headers with 100 mil pin spacing and 28 mil holes.\n\nThese may be from an obsolete version of the library \"th_header.fpl\" with holes that are too small for standard parts. Please check your design." );
		dlg.DoModal();
		g_bShow_header_28mil_hole_warning = !dlg.bDontShowBoxState;
	}
	return bHeaders_28mil_holes;
}


