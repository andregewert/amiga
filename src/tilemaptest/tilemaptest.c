/*
 * Copyright (c) 2024 André Gewert <agewert@ubergeek.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/guigfx.h>
#include <proto/exec.h>
#include <graphics/modeid.h>
#include <intuition/intuition.h>
#include <clib/dos_protos.h>

#define _debug(txt) fprintf(stderr, txt)

void init();
void shutdown();
//void loadTilemap();
//void drawTilemap();
//void drawAnimatedTiles();
//void drawActors();
//void updateGame();

struct Screen* screen;
struct Library* GuiGFXBase;
struct BitMap* TileMap;
struct BitMap* TileMapLayer;
struct BitMap* PlayfieldLayer;

struct Tilemap {
    struct BitMap* bitmap;
    int tileLength;
    int countX;
    int countY;
};

int main(int argc, char* argv[]) {
    init();
    Delay(500);
    shutdown();
}

void init() {

    // TODO Open libraries

    screen = OpenScreenTags(NULL,
                   SA_Width, 320,
                   SA_Height, 200,
                   SA_Depth, 5,
                   SA_DisplayID, LORES_KEY,
                   SA_ShowTitle, FALSE,
                   SA_Draggable, FALSE,
                   TAG_END);
}

void shutdown() {
    if (screen != NULL) {
        CloseScreen(screen);
    }
}

struct Tilemap* loadTileMap(STRPTR filename) {
    APTR picture = NULL, psm = NULL, drawhandle = NULL;

    struct Tilemap *tilemap = (struct Tilemap *) AllocMem(sizeof(struct Tilemap), MEMF_CLEAR);
    if (tilemap == NULL) {
        _debug("Could not allocate tilemap");
        return NULL;
    }

    picture = LoadPicture(filename, TAG_DONE);
    psm = CreatePenShareMap(TAG_DONE);

    tilemap->bitmap = AllocBitMap(16, 16, 8, BMF_CLEAR, screen->RastPort->BitMap);
    tilemap->tileLength = 16;
    tilemap->countX = 20;
    tilemap->countY = 15;

    return tilemap;
}