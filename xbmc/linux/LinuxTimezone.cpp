/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <time.h>
#include "system.h"
#include "PlatformInclude.h"
#include "LinuxTimezone.h"
#include "Util.h"
#include "getdelim.h"

using namespace std;

CLinuxTimezone::CLinuxTimezone()
{
   char* line = NULL;
   size_t linelen;
   CStdString s;
   vector<CStdString> tokens;
   m_IsDST = 0;
   // Load timezones
   FILE* fp = fopen("/usr/share/zoneinfo/zone.tab", "r");
   if (fp)
   {
      CStdString countryCode;
      CStdString timezoneName;

      while (getdelim(&line, &linelen, '\n', fp) > 0)
      {
         tokens.clear();
         s = line;
         s.TrimLeft(" \t").TrimRight(" \n");

         if (s.length() == 0)
            continue;

         if (s[0] == '#')
            continue;

         CUtil::Tokenize(s, tokens, " \t");
         if (tokens.size() < 3)
            continue;

         countryCode = tokens[0];
         timezoneName = tokens[2];

         if (m_timezonesByCountryCode.count(countryCode) == 0)
         {
            vector<CStdString> timezones;
            timezones.push_back(timezoneName);
            m_timezonesByCountryCode[countryCode] = timezones;
         }
         else
         {
            vector<CStdString>& timezones = m_timezonesByCountryCode[countryCode];
            timezones.push_back(timezoneName);
         }

         m_countriesByTimezoneName[timezoneName] = countryCode;
      }

      fclose(fp);
   }

   // Load countries
   fp = fopen("/usr/share/zoneinfo/iso3166.tab", "r");
   if (fp)
   {
      CStdString countryCode;
      CStdString countryName;

      while (getdelim(&line, &linelen, '\n', fp) > 0)
      {
         s = line;
         s.TrimLeft(" \t").TrimRight(" \n");

         if (s.length() == 0)
            continue;

         if (s[0] == '#')
            continue;

         // Search for the first non space from the 2nd character and on
         int i = 2;
         while (s[i] == ' ' || s[i] == '\t') i++;

         countryCode = s.Left(2);
         countryName = s.Mid(i);

         m_counties.push_back(countryName);
         m_countryByCode[countryCode] = countryName;
         m_countryByName[countryName] = countryCode;
      }

      sort(m_counties.begin(), m_counties.end(), sortstringbyname());

      fclose(fp);
   }
}

vector<CStdString> CLinuxTimezone::GetCounties()
{
   return m_counties;
}

vector<CStdString> CLinuxTimezone::GetTimezonesByCountry(const CStdString country)
{
   return m_timezonesByCountryCode[m_countryByName[country]];
}

CStdString CLinuxTimezone::GetCountryByTimezone(const CStdString timezone)
{
#ifdef __APPLE__
  return CStdString("?");
#else
   return m_countryByCode[m_countriesByTimezoneName[timezone]];
#endif
}

void CLinuxTimezone::SetTimezone(CStdString timezoneName)
{
#ifndef __APPLE__
   static char env_var[255];
   sprintf(env_var, "TZ=:%s", timezoneName.c_str());
   putenv(env_var);
   tzset();
#endif
}

CStdString CLinuxTimezone::GetOSConfiguredTimezone()
{
   char timezoneName[255];
   timezoneName[0] = 0;
   FILE* fp = fopen("/etc/timezone", "r");
   if (fp)
   {
      fgets(timezoneName, sizeof(timezoneName), fp);
      timezoneName[strlen(timezoneName)-1] = '\0';
      fclose(fp);
   }

   return timezoneName;
}

CLinuxTimezone g_timezone;
