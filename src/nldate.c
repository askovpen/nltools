#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "julian.h"
#include "nldate.h"
#include "nllog.h"
#include "nlstring.h"

static char *monthnames[]=
{
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

/* this routine does a relaxed check of the header, thus allowing for
   non-standard othernet lodelist headers */

long parse_nodelist_date(char *filename)
{
    char buffer[300], *cpy, *cpj;
    int sig;
    int year, day;

    FILE *f = fopen(filename, "r");

    if (f == NULL)
    {
        logentry(LOG_ERROR, "cannot open %s", filename);
        return -1L;
    }

    if (fgets(buffer, sizeof(buffer), f) == NULL)
    {
        fclose(f);
        logentry(LOG_ERROR, "I/O error reading %s", filename);
        return -1L;
    }

    fclose(f);

    cpy = cpj = strstr(buffer, " -- ");
    if (cpy == NULL) goto invalid;

    while (cpy != buffer && !isdigit(*cpy)) cpy--;
    if (!isdigit(*cpy)) goto invalid;

    sig = 1; year = 0;
    while (cpy != buffer && isdigit(*cpy))
    {
        year = year + sig * (*cpy - '0');
        cpy --;
        sig*=10;
    }
    if (year < 1980 || year > 9999) goto invalid;

    while (!isdigit(*cpj)) cpj++;
    if (!isdigit(*cpj)) goto invalid;

    day = 0;
    while (isdigit(*cpj))
    {
        day = day * 10 + (*cpj - '0');
        cpj ++;
    }
    if (day < 1 || day > 366) goto invalid;

    return get_julian_date(day, 0, year);

invalid:
    logentry(LOG_ERROR, "invalid header in %s", filename);
    return -1L;
}


#if 0

/* this routine does an exact check of the header. unfortunately, many
   othernet nodelists fail this check */

long parse_nodelist_date(char *filename)
{
    char buffer[300], *cp;
    int m, d, y, dn, dnv;
    long julian;
    
    FILE *f = fopen(filename, "r");

    if (f == NULL)
    {
        logentry(LOG_ERROR, "Cannot open %s.", filename);
        return -1L;
    }

    if (fgets(buffer, sizeof(buffer), f) == NULL)
    {
        fclose(f);
        logentry(LOG_ERROR, "I/O error reading %s.", filename);
        return -1L;
    }

    for (m = 0; m < 12; m++)
    {
        if ((cp = strstr(buffer, monthnames[m])) != NULL)
        {
            m++;
            break;
        }
    }

    if (cp == NULL) goto invalid;

    cp += strlen(monthnames[m-1]);
    if (isspace(*cp)) cp++;

    for (d = 0; isdigit(*cp); cp++)
    {
        d = d*10 + (*cp - '0');
    }

    if (cp[0] != ',' || cp[1] != ' ' || d < 1 || d > 31) goto invalid;

    cp += 2;

    for (y = 0; isdigit(*cp); cp++)
    {
        y = y * 10 + (*cp - '0');
    }

    if (ncasecmp(cp," -- Day number ", 15) || y < 1980) goto invalid;

    cp += 15;

    for (dn = 0; isdigit(*cp); cp++)
    {
        dn = dn * 10 + (*cp - '0');
    }

    if (dn < 1 || dn > 365) goto invalid;

    julian = get_julian_date(d, m, y);
    decode_julian_date(julian, NULL, NULL, NULL, &dnv);

    if (dn != dnv)
    {
        fclose(f);
        logentry(LOG_ERROR, "Inconsistent nodelist header information in %s.",
                 filename);
        return -1L;
    }

    fclose(f);
    return julian;
    
invalid:
    fclose(f);
    logentry(LOG_ERROR, "Invalid nodelist header in %s.", filename);
    return -1L;
}

#endif
