#include "include/wl_def.h"

extern int mapon;
extern maptype	*mapheaderseg[];

automap_t Auto_Map;

static int scale = 12; // (4 to 16)

static int clip_tile(int *rx, int *ry, int *rw, int *rh)
{
    // trivial cases
    if (*rx + *rw < 10)
        return 0; // don't draw tile
    if (*rx > (320-10))
        return 0; // don't draw tile
    if (*ry + *rh < 10)
        return 0; // don't draw tile
    if (*ry > (200-10))
        return 0; // don't draw tile

    if (*rx < 10)
    {
        *rw -= (10 - *rx);
        *rx = 10;
    }
    if (*rx + *rw > (320-10))
        *rw -= ((*rx + *rw) - (320-10));

    if (*ry < 10)
    {
        *rh -= (10 - *ry);
        *ry = 10;
    }
    if (*ry + *rh > (200-10))
        *rh -= ((*ry + *rh) - (200-10));

    return 1; // draw tile
}

static void draw_obj(int x, int y, int a, int c)
{
    if ((x<10) || (x>(320-10)) || (y<10) || (y>(200-10)))
        return;

    if (c == YELLOW)
    {
        // draw item
        VW_Plot(x, y, c);
        VW_Plot(x+1, y, c);
        VW_Plot(x-1, y, c);
        VW_Plot(x, y+1, c);
        VW_Plot(x, y-1, c);
    }
    else
    {
        // draw directional object (player or enemy)
        VW_Plot(x, y, c);
        switch (a)
        {
            case 0:
            VW_Plot(x, y-1, c);
            VW_Plot(x, y+1, c);
            VW_Plot(x+1, y, c);
            break;
            case 1:
            VW_Plot(x-1, y-1, c);
            VW_Plot(x+1, y+1, c);
            VW_Plot(x+1, y-1, c);
            break;
            case 2:
            VW_Plot(x-1, y, c);
            VW_Plot(x+1, y, c);
            VW_Plot(x, y-1, c);
            break;
            case 3:
            VW_Plot(x-1, y+1, c);
            VW_Plot(x+1, y-1, c);
            VW_Plot(x-1, y-1, c);
            break;
            case 4:
            VW_Plot(x, y+1, c);
            VW_Plot(x, y-1, c);
            VW_Plot(x-1, y, c);
            break;
            case 5:
            VW_Plot(x-1, y-1, c);
            VW_Plot(x+1, y+1, c);
            VW_Plot(x-1, y+1, c);
            break;
            case 6:
            VW_Plot(x-1, y, c);
            VW_Plot(x+1, y, c);
            VW_Plot(x, y+1, c);
            break;
            case 7:
            VW_Plot(x+1, y-1, c);
            VW_Plot(x-1, y+1, c);
            VW_Plot(x+1, y+1, c);
            break;
        }
    }
}

void AutoMap(void)
{
    signed char *level = &mapheaderseg[mapon]->name[0];
    int cx = 0;
    int cy = 0;

    // clear screen
    VW_Bar(0, 0, 320, 200, BLACK);
    // update display
    VW_UpdateScreen();
    VW_FadeIn();

	SETFONTCOLOR(WHITE, BLACK);

	IN_ClearKeysDown();
    do
    {
        int i, j, c;
        objtype *ob;
        statobj_t *sob;

        INL_Update();

        if (IN_KeyDown(sc_Y))
        {
            if (IN_KeyDown(sc_UpArrow) || IN_KeyDown(sc_LeftArrow)) {
                scale = (scale < 16) ? scale+1 : 16; // B + Up/Left = scale map smaller
                while (IN_KeyDown(sc_UpArrow) || IN_KeyDown(sc_LeftArrow))
                    INL_Update();
            }
            if (IN_KeyDown(sc_DownArrow) || IN_KeyDown(sc_RightArrow)) {
                scale = (scale > 4) ? scale-1 : 4; // B + Down/Right = scale map larger
                while (IN_KeyDown(sc_DownArrow) || IN_KeyDown(sc_RightArrow))
                    INL_Update();
            }
        }
        else if (IN_KeyDown(sc_Space))
        {
            if (IN_KeyDown(sc_UpArrow)) {
                Auto_Map.walls ^= 1; // C + Up = toggle walls
                while (IN_KeyDown(sc_UpArrow))
                    INL_Update();
            }
            if (IN_KeyDown(sc_DownArrow)) {
                Auto_Map.secrets ^= 1; // C + Down = toggle secrets
                while (IN_KeyDown(sc_DownArrow))
                    INL_Update();
            }
            if (IN_KeyDown(sc_LeftArrow)) {
                Auto_Map.items ^= 1; // C + Left = toggle items
                while (IN_KeyDown(sc_LeftArrow))
                    INL_Update();
            }
            if (IN_KeyDown(sc_RightArrow)) {
                Auto_Map.enemies ^= 1; // C + Right = toggle enemies
                while (IN_KeyDown(sc_RightArrow))
                    INL_Update();
            }
        }
        else
        {
            if (IN_KeyDown(sc_UpArrow))
                cy += 4; // Up = scroll down
            if (IN_KeyDown(sc_DownArrow))
                cy -= 4; // Down = scroll up
            if (IN_KeyDown(sc_LeftArrow))
                cx += 4; // Left = scroll right
            if (IN_KeyDown(sc_RightArrow))
                cx -= 4; // Right = scroll left
        }

        // clear screen
        VW_Bar(0, 0, 320, 200, BLACK);

        // print level name
        WindowX = WindowY = 0;
        WindowW = 320;
        WindowH = 8;
        US_PrintCentered(level);

        // draw tiles
        for (i=0; i<MAPSIZE; i++)
        {
            for (j=0; j<MAPSIZE; j++)
            {
                int rx, ry, rw, rh;

                if (!tilemap[i][j])
                    continue;

                rx = (i<<6)/scale - (player->x>>10)/scale + cx + 320/2;
                ry = (j<<6)/scale - (player->y>>10)/scale + cy + 200/2;
                rw = 64/scale;
                rh = 64/scale;
                if (clip_tile(&rx, &ry, &rw, &rh))
                {
                    c = BLACK;
                    if (tilemap[i][j] < 0x80)
                    {
                        // wall
                        if (Auto_Map.seen[i][j])
                            c = GRAY;
                        else if (Auto_Map.walls)
                            c = DARKGRAY;
                    }
                    if (*(mapsegs[1]+farmapylookup[j]+i) == PUSHABLETILE)
                    {
                        // pushwall - same as wall unless secrets shown
                        if (Auto_Map.seen[i][j])
                            c = GRAY;
                        else if (Auto_Map.walls)
                            c = DARKGRAY;
                        if (Auto_Map.secrets)
                            c = Auto_Map.seen[i][j] ? YELLOW : BROWN;
                    }
                    if ((tilemap[i][j] & 0xC0) == 0x80)
                    {
                        // door
                        if (Auto_Map.seen[i][j])
                            c = LITEBLUE;
                        else if (Auto_Map.walls)
                            c = BLUE;
                    }
                    if (c != BLACK)
                        VW_Bar(rx, ry, rw, rh, c);
                }
            }
        }
        // draw objects
        ob = player;
        draw_obj(cx + 320/2, cy + 200/2, ((ob->angle+22)/45) & 7, WHITE);
        ob = ob->next;
        while (ob)
        {
            if (ob->flags & FL_SHOOTABLE)
                if (Auto_Map.enemies)
                {
                    int x = (ob->x>>10)/scale - (player->x>>10)/scale + cx + 320/2;
                    int y = (ob->y>>10)/scale - (player->y>>10)/scale + cy + 200/2;
                    draw_obj(x, y, ob->dir & 7, RED);
                }
            ob = ob->next;
        }
        sob = &statobjlist[0];
        while (sob != laststatobj)
        {
            if (sob->shapenum != -1)
                if (sob->flags & FL_BONUS)
                    if (Auto_Map.items)
                    {
                        int x = (((int)sob->tilex<<6)+(TILEGLOBAL>>11))/scale - (player->x>>10)/scale + cx + 320/2;
                        int y = (((int)sob->tiley<<6)+(TILEGLOBAL>>11))/scale - (player->y>>10)/scale + cy + 200/2;
                        draw_obj(x, y, 0, YELLOW);
                    }
            sob++;
        }
        // update display
        VW_UpdateScreen();
    } while (!IN_KeyDown(sc_A));

    VW_FadeOut();
    WindowX = WindowY = 0;
    WindowW = 320;
    WindowH = 200;
}
