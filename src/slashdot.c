
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "config.h"
#include "common.h"

/* Our macros */
#define FACTOR 1
//#define FONTNAME "font.png" // use with FACTOR = 2
//#define FONTNAME "font_dilated.png" // use with FACTOR = 2
#define FONTNAME "font_dilated_half.png" // use with FACTOR = 1

/* Global stuff */
char *result;

struct
{
    int xmin, ymin, xmax, ymax;
}
objlist[100];
int objects = 0, first = -1, last = -1;

/* Functions */

void flood_fill(struct image *img, int x, int y, int r, int g, int b)
{
    int oldr, oldg, oldb;
    int nextr, nextg, nextb;

    if(x < 0 || y < 0 || x >= img->width || y >= img->height)
        return;

    getpixel(img, x, y, &oldr, &oldg, &oldb);
    setpixel(img, x, y, r, g, b);

    getpixel(img, x + 1, y, &nextr, &nextg, &nextb);
    if(nextr == oldr && nextg == oldg && nextb == oldb)
        flood_fill(img, x + 1, y, r, g, b);

    getpixel(img, x - 1, y, &nextr, &nextg, &nextb);
    if(nextr == oldr && nextg == oldg && nextb == oldb)
        flood_fill(img, x - 1, y, r, g, b);

    getpixel(img, x, y + 1, &nextr, &nextg, &nextb);
    if(nextr == oldr && nextg == oldg && nextb == oldb)
        flood_fill(img, x, y + 1, r, g, b);

    getpixel(img, x, y - 1, &nextr, &nextg, &nextb);
    if(nextr == oldr && nextg == oldg && nextb == oldb)
        flood_fill(img, x, y - 1, r, g, b);
}

struct image *count_objects(struct image *img)
{
    struct image *dst;
    int gotblack = 1;
    int x, y, i;
    int r, g, b;

    dst = new_image(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, r, g, b);
        }

    while(gotblack)
    {
        gotblack = 0;
        for(y = 0; y < dst->height; y++)
            for(x = 0; x < dst->width; x++)
            {
                getpixel(dst, x, y, &r, &g, &b);
                if(r == 50 && g == 50 && b == 50)
                {
                    gotblack = 1;
                    flood_fill(dst, x, y, 255 - objects, 0, 0);
                    objects++;
                }
            }
    }

    //printf("%i objects\n", objects);

    for(i = 0; i < objects; i++)
    {
        objlist[i].ymin = dst->height;
        objlist[i].ymax = 0;

        for(y = 0; y < dst->height; y++)
            for(x = 0; x < dst->width; x++)
            {
                getpixel(dst, x, y, &r, &g, &b);
                if(r == 255 - i && g == 0 && b == 0)
                {
                    if(y < objlist[i].ymin) { objlist[i].ymin = y; objlist[i].xmin = x; }
                    if(y > objlist[i].ymax) { objlist[i].ymax = y; objlist[i].xmax = x; }
                }
            }
        //printf("y min-max: %i %i (size %i)\n", objlist[i].ymin, objlist[i].ymax, objlist[i].ymax - objlist[i].ymin + 1);
        if(objlist[i].ymax - objlist[i].ymin > 18 && objlist[i].ymax - objlist[i].ymin < 27)
        {
            if(first == -1)
                first = i;
            last = i;
            flood_fill(dst, objlist[i].xmin, objlist[i].ymin, 0, 0, 255);
        }
    }

#if 0
    { CvPoint A, B;
      A.x = (objlist[first].xmin + objlist[first].xmax) / 2;
      A.y = (objlist[first].ymin + objlist[first].ymax) / 2;
      B.x = (objlist[last].xmin + objlist[last].xmax) / 2;
      B.y = (objlist[last].ymin + objlist[last].ymax) / 2;
      cvLine(dst, A, B, 0, 2.0, 0);
    }
#endif

    return dst;
}

struct image *rotate(struct image *img)
{
    struct image *dst;
    int x, y, xdest, ydest;
    int r, g, b, R, G, B;
    int X = objlist[first].xmin - objlist[last].xmin;
    int Y = objlist[first].ymin - objlist[last].ymin;
    float xtmp, ytmp;
    float sina = (1.0 * Y) / sqrt(1.0 * X * X + Y * Y);
    float cosa = (1.0 * X) / sqrt(1.0 * X * X + Y * Y);
    if(sina * cosa > 0)
    {
        sina = -sina;
        cosa = -cosa;
    }

    dst = new_image(img->width * FACTOR, img->height * FACTOR);

    for(y = 0; y < img->height * FACTOR; y++)
        for(x = 0; x < img->width * FACTOR; x++)
        {
            xtmp = 1.0 * (x - img->width * FACTOR / 2) / FACTOR;
            ytmp = 1.0 * (y - img->height * FACTOR / 2) / FACTOR;
            xdest = xtmp * cosa - ytmp * sina + 0.5 * img->width;
            ydest = ytmp * cosa + xtmp * sina + 0.5 * img->height;
            //R = G = B = 0;
            getpixel(img, xdest, ydest, &r, &g, &b);
            //R += r; G += g; B += b;
            //getpixel(img, xdest+1, ydest, &r, &g, &b);
            //R += r; G += g; B += b;
            //getpixel(img, xdest, ydest+1, &r, &g, &b);
            //R += r; G += g; B += b;
            //getpixel(img, xdest+1, ydest+1, &r, &g, &b);
            //R += r; G += g; B += b;
            //r = R / 4; g = G / 4; b = B / 4;
            if(r == 255 && g == 0 && b == 255)
                g = 255;
            setpixel(dst, x, y, r, g, b);
        }

    return dst;
}

struct image *fill_holes(struct image *img)
{
    struct image *dst;
    int x, y;
    int r, g, b;

    dst = new_image(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, r, g, b);
        }

    for(y = 0; y < dst->height; y++)
        for(x = 2; x < dst->width - 2; x++)
        {
            int c1, c2, c3, c4, c5;
            getpixel(img, x-2, y, &c1, &g, &b);
            getpixel(img, x-1, y, &c2, &g, &b);
            getpixel(img, x, y, &c3, &g, &b);
            getpixel(img, x+1, y, &c4, &g, &b);
            getpixel(img, x+2, y, &c5, &g, &b);
            if(c1 < 127 && c2 < 127 && c3 > 128 && c4 < 127)
                c3 = (c1 + c2 + c4) / 3;
            else if(c2 < 127 && c3 > 128 && c4 < 127 && c5 < 127)
                c3 = (c2 + c4 + c5) / 3;
            setpixel(dst, x, y, c3, c3, c3);
        }

    for(x = 0; x < dst->width; x++)
        for(y = 2; y < dst->height - 2; y++)
        {
            int c1, c2, c3, c4, c5;
            getpixel(img, x, y-2, &c1, &g, &b);
            getpixel(img, x, y-1, &c2, &g, &b);
            getpixel(img, x, y, &c3, &g, &b);
            getpixel(img, x, y+1, &c4, &g, &b);
            getpixel(img, x, y+2, &c5, &g, &b);
            if(c1 < 127 && c2 < 127 && c3 > 128 && c4 < 127)
                c3 = (c1 + c2 + c4) / 3;
            else if(c2 < 127 && c3 > 128 && c4 < 127 && c5 < 127)
                c3 = (c2 + c4 + c5) / 3;
            setpixel(dst, x, y, c3, c3, c3);
        }

    return dst;
}

struct image *cut_cells(struct image *img)
{
    struct image *dst;
    int x, y;
    int r, g, b;

    dst = new_image(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, r, g, b);
        }

    for(x = 0; x < img->width; x++)
    {
        setpixel(dst, x, 0, 255, 255, 255);
        setpixel(dst, x, img->height - 1, 255, 255, 255);
    }

    for(y = 0; y < img->height; y++)
        for(x = 0; x < 7; x++)
        {
            setpixel(dst, x * img->width / 7, y, 255, 255, 255);
            setpixel(dst, (x + 1) * img->width / 7 - 1, y, 255, 255, 255);
        }

    return dst;
}

struct image *detect_lines(struct image *img)
{
    struct image *dst;
    int x, y;
    int r, ra, rb, g, b;

    dst = new_image(img->width, img->height);

    /* Remove white lines */
    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, r, g, b);
#if 1
            if(y > 0 && y < img->height - 1)
            {
                getpixel(img, x, y - 1, &ra, &g, &b);
                getpixel(img, x, y + 1, &rb, &g, &b);
                if(r > ra && (r - ra) * (r - rb) > 5000)
                    setpixel(dst, x, y, ra, ra, ra);
            }
#endif
        }

    /* Remove black lines */
    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(dst, x, y, &r, &g, &b);
            if(y > 0 && y < img->height - 1)
            {
                getpixel(img, x, y - 1, &ra, &g, &b);
                getpixel(img, x, y + 1, &rb, &g, &b);
                if(r < ra && (r - ra) * (r - rb) > 500)
                    setpixel(dst, x, y, ra, ra, ra);
            }
        }

    return dst;
}

struct image *equalize(struct image *img)
{
    struct image *dst;
    int x, y;
    int r, g, b;

    dst = new_image(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            if(r < 200) r = 50; else r = 200;
            setpixel(dst, x, y, r, r, r);
        }

    return dst;
}

struct image *trick(struct image *img)
{
#define TSIZE 3
    struct image *dst;
    int x, y, i, j, val, m, more, l, less;
    int r, g, b;

    dst = new_image(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
            setpixel(dst, x, y, 255, 255, 255);

    for(y = TSIZE/2; y < img->height - TSIZE/2; y++)
        for(x = TSIZE/2; x < img->width - TSIZE/2; x++)
        {
            getpixel(img, x + j - TSIZE/2, y + i - TSIZE/2, &val, &g, &b);
            m = more = l = less = 0;
            for(i = 0; i < TSIZE; i++)
                for(j = 0; j < TSIZE; j++)
                {
                    getpixel(img, x + j - TSIZE/2, y + i - TSIZE/2, &r, &g, &b);
                    if(r > val)
                    {
                        more += r;
                        m++;
                    }
                    else if(r < val)
                    {
                        less += r;
                        l++;
                    }
                }

            if(l >= 6)
                i = less / l;
            else if(m >= 6)
                i = more / m;
            else
                i = val;
            setpixel(dst, x, y, i, i, i);
        }

    return dst;
}

struct image *smooth(struct image *img)
{
#define SSIZE 3
    struct image *dst;
    int x, y, i, j, val;
    int r, g, b;

    dst = new_image(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
            setpixel(dst, x, y, 255, 255, 255);
return dst;

    for(y = SSIZE/2; y < img->height - SSIZE/2; y++)
        for(x = SSIZE/2; x < img->width - SSIZE/2; x++)
        {
            val = 0;
            for(i = 0; i < SSIZE; i++)
                for(j = 0; j < SSIZE; j++)
                {
                    getpixel(img, x + j - SSIZE/2, y + i - SSIZE/2, &r, &g, &b);
                    val += r;
                }

            i = val / (SSIZE * SSIZE);
            setpixel(dst, x, y, i, i, i);
        }

    return dst;
}

struct image *median(struct image *img)
{
#define MSIZE 4
    struct image *dst;
    int x, y, i, j, val[MSIZE*MSIZE];
    int r, g, b;

    dst = new_image(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
            setpixel(dst, x, y, 255, 255, 255);

    for(y = MSIZE/2; y < img->height - MSIZE/2; y++)
        for(x = MSIZE/2; x < img->width - MSIZE/2; x++)
        {
            for(i = 0; i < MSIZE; i++)
                for(j = 0; j < MSIZE; j++)
                {
                    getpixel(img, x + j - SSIZE/2, y + i - SSIZE/2, &r, &g, &b);
                    val[i * MSIZE + j] = r;
                }

            /* Bubble sort power! */
            for(i = 0; i < MSIZE * MSIZE / 2 + 1; i++)
                for(j = i + 1; j < MSIZE * MSIZE; j++)
                    if(val[i] > val[j])
                    {
                        register int k = val[i];
                        val[i] = val[j];
                        val[j] = k;
                    }

            i = val[MSIZE * MSIZE / 2];
            setpixel(dst, x, y, i, i, i);
        }

    return dst;
}

struct image * find_glyphs(struct image *img)
{
    char all[] = "abcdefgijkmnpqrstvwxyz";
    struct
    {
        int xmin, xmax, ymin, ymax;
        int count;
    }
    glyphs[22];
    struct image *dst;
    struct image *font = load_image(FONTNAME);
    int x, y, i = 0;
    int r, g, b, r2, g2, b2;
    int xmin, xmax, ymin, ymax, incell = 0, count = 0, startx = 0, cur = 0;
    int distmin, distx, disty, distch;

    if(!font)
    {
        fprintf(stderr, "cannot load font %s\n", FONTNAME);
        exit(-1);
    }

    dst = new_image(img->width, img->height);

    for(y = 0; y < img->height; y++)
        for(x = 0; x < img->width; x++)
        {
            getpixel(img, x, y, &r, &g, &b);
            setpixel(dst, x, y, 255, g, 255);
        }

    strcpy(result, "       ");

    for(x = 0; x < font->width; x++)
    {
        int found = 0;
        for(y = 0; y < font->height; y++)
        {
            getpixel(font, x, y, &r, &g, &b);
            if(r < 128)
            {
                found = 1;
                count += (255 - r);
            }
        }
        if(found && !incell)
        {
            incell = 1;
            xmin = x;
        }
        else if(!found && incell)
        {
            incell = 0;
            xmax = x;
#if 0
            ymin = font->height;
            ymax = 0;
            for(y = 0; y < font->height; y++)
            {
                int newx;
                int gotit = 0;
                for(newx = xmin; newx < xmax; newx++)
                {
                    getpixel(font, newx, y, &r, &g, &b);
                    if(r < 128)
                    {
                        gotit = 1;
                        break;
                    }
                }
                if(gotit)
                {
                    if(ymin > y) ymin = y;
                    if(ymax <= y) ymax = y + 1;
                }
            }
#else
            ymin = 0;
            ymax = font->height;
#endif
            glyphs[i].xmin = xmin;
            glyphs[i].xmax = xmax;
            glyphs[i].ymin = ymin;
            glyphs[i].ymax = ymax;
            glyphs[i].count = count;
            count = 0;
            i++;
        }
    }

    if(i != 22)
    {
        printf("error: could not find 22 glyphs in font\n");
        exit(-1);
    }

while(cur < 7)
{
    /* Try to find 1st letter */
    distmin = 999999999;
    for(i = 0; i < 22; i++)
    {
        int localmin = 99999999, localx, localy;
//if(all[i] == 'i') continue;
        xmin = glyphs[i].xmin;
        ymin = glyphs[i].ymin;
        xmax = glyphs[i].xmax;
        ymax = glyphs[i].ymax;
        //printf("trying to find %c (%i×%i) - ", all[i], xmax - xmin, ymax - ymin);
        for(y = -5 * FACTOR; y < 5 * FACTOR; y++)
            for(x = startx - 5 * FACTOR; x < startx + 5 * FACTOR; x++)
            {
                int z, t, dist;
                dist = 0;
                for(t = 0; t < ymax - ymin; t++)
                    for(z = 0; z < xmax - xmin; z++)
                    {
                        getgray(font, xmin + z, ymin + t, &r);
                        getgray(img, x + z, y + t, &r2);
                        dist += abs(r - r2);
                    }
//                printf("%i %i -> %i\n", x, y, dist);
                //dist /= sqrt(xmax - xmin);
                dist = dist * 128 / glyphs[i].count;
                if(dist < localmin)
                {
                    localmin = dist;
                    localx = x;
                    localy = y;
                }
            }
        //fprintf(stderr, "%i (%i,%i)\n", localmin, localx - startx, localy);
        if(localmin < distmin)
        {
            distmin = localmin;
            distx = localx;
            disty = localy;
            distch = i;
        }
    }

    //fprintf(stderr, "%i (%i,%i)\n", distmin, distx - startx, disty);
    //printf("min diff: %c - %i (%i, %i)\n", all[distch], distmin, distx, disty);

    /* Print min glyph */
    xmin = glyphs[distch].xmin;
    ymin = glyphs[distch].ymin;
    xmax = glyphs[distch].xmax;
    ymax = glyphs[distch].ymax;
    for(y = 0; y < ymax - ymin; y++)
        for(x = 0; x < xmax - xmin; x++)
        {
            getpixel(font, xmin + x, ymin + y, &r, &g, &b);
            if(r > 128) continue;
            setpixel(dst, distx + x, disty + y, r, g, b);
        }

    startx = distx + xmax - xmin;
    result[cur++] = all[distch];
}

    return dst;
}

char * slashdot_decode(char *image)
{
    struct image *img, *tmp, *tmp2, *dst;

    img = load_image(image);
    if(img == NULL)
        return NULL;

    result = malloc(8 * sizeof(char));
//    display(img);

//    tmp = equalize(img);
//    display(tmp);
//    tmp = fill_holes(tmp);
//    display(tmp);

//    dst = median(tmp);
tmp = smooth(img);
tmp = fill_holes(img);
tmp = median(tmp);
//tmp = smooth(tmp);
//display(tmp);
//tmp = trick(tmp);
//display(tmp);
tmp = equalize(tmp);
//display(tmp);

tmp = detect_lines(img);
tmp = fill_holes(tmp);
//tmp = cut_cells(tmp);
//display_image(tmp);

tmp2 = median(tmp);
tmp2 = equalize(tmp2);
count_objects(tmp2);
//display_image(tmp2);

//tmp = median(tmp);
tmp = rotate(tmp);
tmp = median(tmp);
//display_image(tmp);
//tmp = equalize(tmp);
//tmp = cut_cells(tmp);
//        cvSaveImage(argv[2], tmp);

tmp = find_glyphs(tmp);
//display_image(tmp);

//        cvSaveImage(argv[3], tmp);

    return result;
}

