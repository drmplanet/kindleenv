describe("Dbg module", function()
    local dbg
    setup(function()
        package.path = "?.lua;common/?.lua;rocks/share/lua/5.1/?.lua;frontend/?.lua;" .. package.path
        dbg = require("dbg")
    end)

    it("setup mt.__call and guard after tunrnOn is called", function()
        local old_call = getmetatable(dbg).__call
        local old_guard = dbg.guard
        dbg:turnOn()
        assert.is_not.same(old_call, getmetatable(dbg).__call)
        assert.is_not.same(old_guard, dbg.guard)
        dbg:turnOff()
    end)

    it("should call pre_gard callback", function()
        local called = false
        local foo = {}
        function foo:bar() end
        assert.is.falsy(called)

        dbg:turnOff()
        assert.is.falsy(called)

        dbg:turnOn()
        dbg:guard(foo, 'bar', function() called = true end)
        foo:bar()
        assert.is.truthy(called)
        dbg:turnOff()
    end)

    it("should call post_gard callback", function()
        local called = false
        local foo = {}
        function foo:bar() end
        assert.is.falsy(called)

        dbg:turnOff()
        assert.is.falsy(called)

        dbg:turnOn()
        dbg:guard(foo, 'bar', nil, function() called = true end)
        foo:bar()
        assert.is.truthy(called)
        dbg:turnOff()
    end)

    it("should return all values returned by the guarded function", function()
        local called = false, re
        local foo = {}
        function foo:bar() return 1 end
        assert.is.falsy(called)

        dbg:turnOn()
        dbg:guard(foo, 'bar', function() called = true end)
        re = {foo:bar()}
        assert.is.truthy(called)
        assert.is.same(re, {1})

        called = false
        function foo:bar() return 1, 2, 3 end
        dbg:guard(foo, 'bar', function() called = true end)
        assert.is.falsy(called)
        re = {foo:bar()}
        assert.is.same(re, {1, 2, 3})
        dbg:turnOff()
    end)
end)
