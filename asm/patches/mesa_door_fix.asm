; Mesa's door is a special case where the entrance is locked by time of day
; This can be an issue if it leads to the Windfall cafe, since some checks
; (Linda and Anton) are only accessible during the day
; The logic system isn't set up to deal with this currently, so for now
; just change the door to be unlocked at all times when doors and randomized
; TODO: replace this with a logic-based solution at some point
.org 0x021A4B30 ; in daKnob00_c::chkException
    beq 0x021A4C28
