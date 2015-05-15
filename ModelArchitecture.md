# Introduction #

The PCB document is the model in the MVC architecture.  It represents the structure of the printed circuit board.

# FreePCB model #

FreePCB models the document using lists of various PCB objects:
  * Part list
    * Part: instance of PCB footprint; has a refdes and value (not implemented using text objects)
      * Part pin: a unique pin in the netlist; attached to a part instance.
  * Net list
    * Nets: represent logical connections between part pins
      * Areas: polygons on copper layers
      * Connects: represents a connection between two pins
        * Segments: trace connecting two vertices
        * Vertices: points where two straight traces join; also act as vias and tees
  * Text list: represents text added to the PCB by the user (not refdes/value strings)
  * Footprint cache: stores footprint descriptions for the footprints currently in use
    * Footprint (CShape)
      * Padstacks
      * Pins
      * Polylines (outline)
      * Glue dots
      * Text list

# Issues #
The above architecture results in some problems:
  * Multiple types of traces are needed, depending on the context (stub vs. pin-to-pin)
  * Connectivity is ignored when computing ratlines: a ratline is generated even when the pins are already connected (e.g. when a stub trace or a copper area connects the pins); ratlines do not connect to nearest copper, but always go to a pin.
  * It is not possible to route traces without first connecting them to either a pin or another trace.

The fundamental problem is that a connection is strictly between two pins.  This is generally not the correct model.  For example, it ignores copper areas, requiring numerous hacks to accomodate them.  In addition, a specific procedure must be followed to connect a pin to an existing trace, instead of simply drawing the appropriate line segments.

Another big problem is handling netlist changes.  For example, if a net is deleted on the schematic, the net naming could shift.  In current versions of FreePCB, this results in all of the affected traces being ripped up when the new netlist is imported.  This is a big problem, especially when changes must be made when a board is completely routed.  While there are workarounds, they involve manual editing of the netlist, which can introduce errors.

A somewhat less serious problem is that segments and vertices are children of nets.  This makes it impossible to create copper traces and areas that are not connected to any net.  It is often necessary to create unconnected copper for mechanical or other reasons (e.g. heatsinks; microwave antenna structures).

# Alternative Architecture #
One way to address this is to decouple Nets and Segments/Vertices/Areas.  Nets are simply sets of pins that must be electrically connected.  The actual connectivity should be dynamically computed from the physical layout of copper traces and polygons.  Physical elements should be completely independent from Nets; for example, a trace should allow pins belonging to different nets to be connected.  While this would generally be an error, it is important to have this flexibility on the architectural level.  A Net object could then verify connectivity by checking that all of its pins are connected together, and that no other pins are attached; ratlines should be drawn in the case of unconnected pins, and short-circuited pins should be highlighted as a DRC error.