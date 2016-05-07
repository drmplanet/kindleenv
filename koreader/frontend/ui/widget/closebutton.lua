--[[--
Button widget that shows an "×" and handles closing window when tapped

Example:

    local CloseButton = require("ui/widget/closebutton")
    local parent_widget = OverlapGroup:new{}
    table.insert(parent_widget, CloseButton:new{
        window = parent_widget,
    })
    UIManager:show(parent_widget)

]]

local InputContainer = require("ui/widget/container/inputcontainer")
local FrameContainer = require("ui/widget/container/framecontainer")
local HorizontalGroup = require("ui/widget/horizontalgroup")
local HorizontalSpan = require("ui/widget/horizontalspan")
local TextWidget = require("ui/widget/textwidget")
local GestureRange = require("ui/gesturerange")
local Screen = require("device").screen
local Font = require("ui/font")

local CloseButton = InputContainer:new{
    overlap_align = "right",
    window = nil,
}

function CloseButton:init()
    local text_widget = TextWidget:new{
        text = "×",
        face = Font:getFace("cfont", 32),
    }
    local padding_span = HorizontalSpan:new{ width = Screen:scaleBySize(14) }

    self[1] = FrameContainer:new{
        bordersize = 0,
        padding = 0,
        HorizontalGroup:new{
            padding_span,
            text_widget,
            padding_span,
        }
    }

    self.ges_events.Close = {
        GestureRange:new{
            ges = "tap",
            -- x and y coordinates for the widget is only known after the it is
            -- drawn. so use callback to get range at runtime.
            range = function() return self.dimen end,
        },
        doc = "Tap on close button",
    }
end

function CloseButton:onClose()
    self.window:onClose()
    return true
end

return CloseButton
