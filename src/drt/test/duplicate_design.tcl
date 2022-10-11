set block [[[ord::get_db] getChip] getBlock]

set inst_count 0
foreach inst [$block getInsts] {
    set origin [$inst getLocation]
    set new_x [expr [lindex $origin 0] + $deltaX]
    set new_y [expr [lindex $origin 1] + $deltaY]
    set old_name [$inst getName]
    set name "${old_name}_dup_${deltaX}_${deltaY}"
    odb::dbInst_duplicate $inst $name $deltaX $deltaY
    set inst_count [expr $inst_count + 1]
    if {[expr $inst_count % 100000] == 0 && $inst_count != 0} {
        puts "\[MIRROR\] Copied ${inst_count} components"
    }
}

foreach net [$block getNets] {
    set name [$net getName]
    set name "${name}_dup_${deltaX}_${deltaY}"
    set new_net [odb::dbNet_create $block $name]
    set iterms [lreverse [$net getITerms]]
    foreach iterm $iterms {
        set inst [$iterm getInst]
        set inst_name [$inst getName]
        set newInst [$block findInst "${inst_name}_dup_${deltaX}_${deltaY}"]
        set newiterm [$newInst findITerm [[$iterm getMTerm] getName]]
        $newiterm connect $new_net
    }
    foreach guide [$net getGuides] {
        set layer [$guide getLayer]
        set box [$guide getBox]
        $box moveDelta $deltaX $deltaY
        odb::dbGuide_create $new_net $layer $box
    }
}
foreach term [$block getBTerms] {
    set net_name [[$term getNet] getName]
    set net_name "${net_name}_dup_${deltaX}_${deltaY}"
    set net [$block findNet $net_name]
    set term_name [$term getName]
    set term_name "${term_name}_dup_${deltaX}_${deltaY}"
    set newTerm [odb::dbBTerm_create $net $term_name]
    $newTerm setIoType [$term getIoType]
    $newTerm setSigType [$term getSigType]
    foreach pin [$term getBPins] {
        set newPin [odb::dbBPin_create $newTerm]
        $newPin setPlacementStatus [$pin getPlacementStatus]
        foreach box [$pin getBoxes] {
            set x1 [expr [$box xMin] + $deltaX]
            set y1 [expr [$box yMin] + $deltaY]
            set x2 [expr [$box xMax] + $deltaX]
            set y2 [expr [$box yMax] + $deltaY]
            set newBox [odb::dbBox_create $newPin [$box getTechLayer] $x1 $y1 $x2 $y2] 
        }
    }
}

foreach grid [$block getTrackGrids] {
    set layer [$grid getTechLayer]
    if {$deltaY == 0} {
        set sz [$grid getNumGridPatternsX]
        for {set i 0} {$i < $sz} {incr i} {
            set result [$grid getGridPatternX $i]
            $grid addGridPatternX [expr [lindex $result 0] + $deltaX] [lindex $result 1] [lindex $result 2]
        }
    } else {
        set sz [$grid getNumGridPatternsY]
        for {set i 0} {$i < $sz} {incr i} {
            set result [$grid getGridPatternY $i]
            $grid addGridPatternY [expr [lindex $result 0] + $deltaY] [lindex $result 1] [lindex $result 2]
        }
    }
}
set die_area [$block getDieArea]
$die_area set_xhi [expr [$die_area xMax] + $deltaX]
$die_area set_yhi [expr [$die_area yMax] + $deltaY]
$block setDieArea $die_area