describe("Readerhighlight module", function()
    local DocumentRegistry, ReaderUI, UIManager, Screen, Geom, dbg
    setup(function()
        require("commonrequire")
        DocumentRegistry = require("document/documentregistry")
        ReaderUI = require("apps/reader/readerui")
        UIManager = require("ui/uimanager")
        Screen = require("device").screen
        Geom = require("ui/geometry")
        dbg = require("dbg")
    end)

    local function highlight_single_word(readerui, pos0)
        readerui.highlight:onHold(nil, { pos = pos0 })
        readerui.highlight:onHoldRelease()
        readerui.highlight:onHighlight()
        UIManager:scheduleIn(1, function()
            UIManager:close(readerui.dictionary.dict_window)
            UIManager:close(readerui)
        end)
        UIManager:run()
    end
    local function highlight_text(readerui, pos0, pos1)
        readerui.highlight:onHold(nil, { pos = pos0 })
        readerui.highlight:onHoldPan(nil, { pos = pos1 })
        readerui.highlight:onHoldRelease()
        assert.truthy(readerui.highlight.highlight_dialog)
        readerui.highlight:onHighlight()
        UIManager:scheduleIn(1, function()
            UIManager:close(readerui.highlight.highlight_dialog)
            UIManager:close(readerui)
        end)
        UIManager:run()
    end
    local function tap_highlight_text(readerui, pos0, pos1, pos2)
        readerui.highlight:onHold(nil, { pos = pos0 })
        readerui.highlight:onHoldPan(nil, { pos = pos1 })
        readerui.highlight:onHoldRelease()
        readerui.highlight:onHighlight()
        readerui.highlight:clear()
        UIManager:close(readerui.highlight.highlight_dialog)
        readerui.highlight:onTap(nil, { pos = pos2 })
        assert.truthy(readerui.highlight.edit_highlight_dialog)
        UIManager:nextTick(function()
            UIManager:close(readerui.highlight.edit_highlight_dialog)
            UIManager:close(readerui)
        end)
        UIManager:run()
    end

    describe("highlight for EPUB documents", function()
        local page = 10
        local readerui
        setup(function()
            local sample_epub = "spec/front/unit/data/juliet.epub"
            readerui = ReaderUI:new{
                document = DocumentRegistry:openDocument(sample_epub),
            }
        end)
        before_each(function()
            UIManager:quit()
            readerui.rolling:onGotoPage(page)
            UIManager:show(readerui)
            -- HACK: Mock UIManager:run x and y for readerui.dimen
            -- TODO: refactor readerview's dimen handling so we can get rid of
            -- this workaround
            readerui:paintTo(Screen.bb, 0, 0)
        end)
        after_each(function()
            readerui.highlight:clear()
        end)
        it("should highlight single word", function()
            highlight_single_word(readerui, Geom:new{ x = 260, y = 80 })
            Screen:shot("screenshots/reader_highlight_single_word_epub.png")
            assert.truthy(readerui.view.highlight.saved[page])
        end)
        it("should highlight text", function()
            highlight_text(readerui,
                           Geom:new{ x = 260, y = 60 },
                           Geom:new{ x = 260, y = 90 })
            Screen:shot("screenshots/reader_highlight_text_epub.png")
            assert.truthy(readerui.view.highlight.saved[page])
        end)
        it("should response on tap gesture", function()
            tap_highlight_text(readerui,
                               Geom:new{ x = 62, y = 374 },
                               Geom:new{ x = 484, y = 374 },
                               Geom:new{ x = 331, y = 374 })
            Screen:shot("screenshots/reader_tap_highlight_text_epub.png")
        end)
    end)

    describe("highlight for PDF documents", function()
        local readerui
        setup(function()
            local sample_pdf = "spec/front/unit/data/sample.pdf"
            readerui = ReaderUI:new{
                document = DocumentRegistry:openDocument(sample_pdf),
            }
        end)
        describe("for scanned page with text layer", function()
            before_each(function()
                UIManager:quit()
                UIManager:show(readerui)
                readerui.paging:onGotoPage(10)
            end)
            after_each(function()
                readerui.highlight:clear()
            end)
            it("should highlight single word", function()
                highlight_single_word(readerui, Geom:new{ x = 260, y = 70 })
                Screen:shot("screenshots/reader_highlight_single_word_pdf.png")
            end)
            it("should highlight text", function()
                highlight_text(readerui, Geom:new{ x = 260, y = 70 }, Geom:new{ x = 260, y = 150 })
                Screen:shot("screenshots/reader_highlight_text_pdf.png")
            end)
            it("should response on tap gesture", function()
                tap_highlight_text(readerui,
                                   Geom:new{ x = 260, y = 70 },
                                   Geom:new{ x = 260, y = 150 },
                                   Geom:new{ x = 280, y = 110 })
                Screen:shot("screenshots/reader_tap_highlight_text_pdf.png")
            end)
        end)
        describe("for scanned page without text layer", function()
            before_each(function()
                UIManager:quit()
                UIManager:show(readerui)
                readerui.paging:onGotoPage(28)
            end)
            after_each(function()
                readerui.highlight:clear()
            end)
            it("should highlight single word", function()
                highlight_single_word(readerui, Geom:new{ x = 260, y = 70 })
                Screen:shot("screenshots/reader_highlight_single_word_pdf_scanned.png")
            end)
            it("should highlight text", function()
                highlight_text(readerui, Geom:new{ x = 260, y = 70 }, Geom:new{ x = 260, y = 150 })
                Screen:shot("screenshots/reader_highlight_text_pdf_scanned.png")
            end)
            it("should response on tap gesture", function()
                tap_highlight_text(readerui, Geom:new{ x = 260, y = 70 }, Geom:new{ x = 260, y = 150 }, Geom:new{ x = 280, y = 110 })
                Screen:shot("screenshots/reader_tap_highlight_text_pdf_scanned.png")
            end)
        end)
        describe("for reflowed page", function()
            before_each(function()
                UIManager:quit()
                readerui.document.configurable.text_wrap = 1
                UIManager:show(readerui)
                readerui.paging:onGotoPage(31)
            end)
            after_each(function()
                readerui.highlight:clear()
                readerui.document.configurable.text_wrap = 0
            end)
            it("should highlight single word", function()
                highlight_single_word(readerui, Geom:new{ x = 260, y = 70 })
                Screen:shot("screenshots/reader_highlight_single_word_pdf_reflowed.png")
            end)
            it("should highlight text", function()
                highlight_text(readerui, Geom:new{ x = 260, y = 70 }, Geom:new{ x = 260, y = 150 })
                Screen:shot("screenshots/reader_highlight_text_pdf_reflowed.png")
            end)
            it("should response on tap gesture", function()
                tap_highlight_text(readerui, Geom:new{ x = 260, y = 70 }, Geom:new{ x = 260, y = 150 }, Geom:new{ x = 280, y = 110 })
                Screen:shot("screenshots/reader_tap_highlight_text_pdf_reflowed.png")
            end)
        end)
    end)
end)