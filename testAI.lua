--print("Hello from Lua!")

local running = true
while running do
    ::continue::


    if getMyTurn() >= 2.0 then
        clearMyTurn()
        addMyTurn(2.0)
    end

    if getMyTurn() >= 1.2 and getEntityCode() ~= 5 and isHostile() then
        if math.abs(playerX()-selfX())>1 or math.abs(playerY()-selfY()) > 1 then
            useMyTurn(1.2)
            local toDir = -1
            local x1 = selfX()
            local y1 = selfY()
            local x2 = playerX()
            local y2 = playerY()
            local xo = x1
            local yo = y1
            local delx = math.abs(x2-x1)
            local dely = math.abs(y2-y1)

            local fabs = math.abs

            if fabs(1.0 * dely / delx) < 1 then
                local p = 2 * dely - delx
                if p < 0 then
                    if x2 > xo and y2 >= yo then
                        x1 = x1 + 1
                    elseif x2 > xo and yo > y2 then
                        x1 = x1 + 1
                    elseif xo > x2 and y2 > yo then
                        x1 = x1 - 1
                    else
                        x1 = x1 - 1
                    end
                else
                    if x2 > xo and y2 >= yo then
                        x1 = x1 + 1
                        y1 = y1 + 1
                    elseif x2 > xo and yo > y2 then
                        x1 = x1 + 1
                        y1 = y1 - 1
                    elseif xo > x2 and y2 > yo then
                        x1 = x1 - 1
                        y1 = y1 + 1
                    else
                        x1 = x1 - 1
                        y1 = y1 - 1
                    end
                end
            elseif fabs(1.0 * dely / delx) > 1 then
                local p = (2 * delx) - dely
                if p < 0 then
                    if x2 >= xo and y2 > yo then
                        y1 = y1 + 1
                    elseif x2 > xo and yo > y2 then
                        y1 = y1 - 1
                    elseif xo > x2 and y2 > yo then
                        y1 = y1 + 1
                    else
                        y1 = y1 - 1
                    end
                else
                    if x2 >= xo and y2 > yo then
                        x1 = x1 + 1
                        y1 = y1 + 1
                    elseif x2 > xo and yo > y2 then
                        x1 = x1 + 1
                        y1 = y1 - 1
                    elseif xo > x2 and y2 > yo then
                        x1 = x1 - 1
                        y1 = y1 + 1
                    else
                        x1 = x1 - 1
                        y1 = y1 - 1
                    end
                end
            else
                if x2 > x1 and y2 > y1 then
                    x1 = x1 + 1
                    y1 = y1 + 1
                elseif x2 > x1 and y1 > y2 then
                    x1 = x1 + 1
                    y1 = y1 - 1
                elseif x1 > x2 and y2 > y1 then
                    x1 = x1 - 1
                    y1 = y1 + 1
                else
                    x1 = x1 - 1
                    y1 = y1 - 1
                end
            end
            
            local dx = x1 - xo
            local dy = y1 - yo
            if dx == 1 and dy == 0 then
                toDir = 0
            elseif dx == 1 and dy == -1 then
                toDir = 1
            elseif dx == 0 and dy == -1 then
                toDir = 2
            elseif dx == -1 and dy == -1 then
                toDir = 3
            elseif dx == -1 and dy == 0 then
                toDir = 4
            elseif dx == -1 and dy == 1 then
                toDir = 5
            elseif dx == 0 and dy == 1 then
                toDir = 6
            else
                toDir = 7
            end
            

            updateWalkable(selfX() + dx, selfY() + dy);
            if isWalkableTile(selfX()+dx,selfY()+dy,selfZ()) then
                if noFogTile(selfX(),selfY(),selfZ()) or noFogTile(selfX()+dx,selfY()+dy,selfZ()) then
                    setDirection(toDir);
                    moveStart(toDir, false);
                    useMyTurn(1.2);
                    return false
                else
                    setDirection(toDir);
                    moveStart(toDir, true);
                end
            end
            goto continue
        end 
    end

    if getMyTurn() >= 1.3 and getEntityCode() ~= 5 and isHostile() then
        if math.abs(playerX()-selfX()) <= 1 and math.abs(playerY()-selfY()) <= 1 then
            useMyTurn(1.3);
            setDirection(coord2Dir(playerX() - selfX(), playerY()-selfY()));
            attackStart(playerX(), playerY(), playerZ());
            return false
        end
    end

    return true
    

end