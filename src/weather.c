/**************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael         *
*  Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                         *
*  In order to use any part of this Merc Diku Mud, you must comply with   *
*  both the original Diku license in 'license.doc' as well the Merc       *
*  license in 'license.txt'.  In particular, you may not remove either of *
*  these copyright notices.                                               *
*                                                                         *
*  Much time and thought has gone into this software and you are          *
*  benefiting.  We hope that you share your changes too.  What goes       *
*  around, comes around.                                                  *
***************************************************************************
*       ROM 2.4 is copyright 1993-1998 Russ Taylor                        *
*       ROM has been brought to you by the ROM consortium                 *
*           Russ Taylor (rtaylor@hypercube.org)                           *
*           Gabrielle Taylor (gtaylor@hypercube.org)                      *
*           Brian Moore (zump@rom.org)                                    *
*       By using this code, you have agreed to follow the terms of the    *
*       ROM license, in the file Rom24/doc/rom.license                    *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/

#include "merc.h"
#include "olc.h"
#include "interp.h"
#include "recycle.h"

Proto (void get_weather_echo, (WeatherData *));


void
init_area_weather ()
{
  AreaData *pArea;

  logf ("Initializing weather data...");
  for (pArea = area_first; pArea; pArea = pArea->next)
    {
      int cf;


      cf = pArea->weather.climate_temp - 2;
      pArea->weather.temp =
	number_range (-mud_info.weath_unit, mud_info.weath_unit) +
	cf * number_range (0, mud_info.weath_unit);
      pArea->weather.temp_vector =
	cf + number_range (-mud_info.rand_factor, mud_info.rand_factor);


      cf = pArea->weather.climate_precip - 2;
      pArea->weather.precip =
	number_range (-mud_info.weath_unit, mud_info.weath_unit) +
	cf * number_range (0, mud_info.weath_unit);
      pArea->weather.precip_vector =
	cf + number_range (-mud_info.rand_factor, mud_info.rand_factor);


      cf = pArea->weather.climate_wind - 2;
      pArea->weather.wind =
	number_range (-mud_info.weath_unit, mud_info.weath_unit) +
	cf * number_range (0, mud_info.weath_unit);
      pArea->weather.wind_vector =
	cf + number_range (-mud_info.rand_factor, mud_info.rand_factor);
    }

  return;
}


void
adjust_vectors (WeatherData * weather)
{
  double dT, dP, dW;

  if (!weather)
    {
      bug ("NULL weather data.");
      return;
    }

  dT = 0;
  dP = 0;
  dW = 0;


  dT += number_range (-mud_info.rand_factor, mud_info.rand_factor);
  dP += number_range (-mud_info.rand_factor, mud_info.rand_factor);
  dW += number_range (-mud_info.rand_factor, mud_info.rand_factor);


  dT += mud_info.climate_factor *
    (((weather->climate_temp - 2) * mud_info.weath_unit) -
     (weather->temp)) / mud_info.weath_unit;
  dP += mud_info.climate_factor *
    (((weather->climate_precip - 2) * mud_info.weath_unit) -
     (weather->precip)) / mud_info.weath_unit;
  dW += mud_info.climate_factor *
    (((weather->climate_wind - 2) * mud_info.weath_unit) -
     (weather->wind)) / mud_info.weath_unit;


  weather->temp_vector += (int) dT;
  weather->precip_vector += (int) dP;
  weather->wind_vector += (int) dW;


  weather->temp_vector = Range (-mud_info.max_vector,
				weather->temp_vector, mud_info.max_vector);
  weather->precip_vector =
    Range (-mud_info.max_vector, weather->precip_vector, mud_info.max_vector);
  weather->wind_vector =
    Range (-mud_info.max_vector, weather->wind_vector, mud_info.max_vector);

  return;
}


void
weather_update ()
{
  AreaData *pArea;
  Descriptor *d;
  int limit;

  limit = 3 * mud_info.weath_unit;

  for (pArea = area_first; pArea; pArea = pArea->next)
    {

      pArea->weather.temp += pArea->weather.temp_vector;
      pArea->weather.precip += pArea->weather.precip_vector;
      pArea->weather.wind += pArea->weather.wind_vector;


      pArea->weather.temp = Range (-limit, pArea->weather.temp, limit);
      pArea->weather.precip = Range (-limit, pArea->weather.precip, limit);
      pArea->weather.wind = Range (-limit, pArea->weather.wind, limit);


      get_weather_echo (&pArea->weather);
    }

  for (pArea = area_first; pArea; pArea = pArea->next)
    {
      adjust_vectors (&pArea->weather);
    }


  for (d = descriptor_first; d; d = d->next)
    {
      WeatherData *weath;

      if (d->connected == CON_PLAYING &&
	  IsOutside (d->character) && IsAwake (d->character))
	{
	  weath = &d->character->in_room->area->weather;
	  if (NullStr (weath->echo))
	    continue;
	  d_printlnf (d, "%s%s{x", weath->echo_color, weath->echo);
	}
    }

  return;
}


void
get_weather_echo (WeatherData * weath)
{
  int n;
  int temp, precip, wind;
  int dT, dP, dW;
  int tindex, pindex, windex;


  replace_str (&weath->echo, "");
  replace_str (&weath->echo_color, "{x");


  n = number_bits (2);


  temp = weath->temp;
  precip = weath->precip;
  wind = weath->wind;

  dT = weath->temp_vector;
  dP = weath->precip_vector;
  dW = weath->wind_vector;

  tindex = (temp + 3 * mud_info.weath_unit - 1) / mud_info.weath_unit;
  pindex = (precip + 3 * mud_info.weath_unit - 1) / mud_info.weath_unit;
  windex = (wind + 3 * mud_info.weath_unit - 1) / mud_info.weath_unit;


  switch (pindex)
    {
    case 0:
      if (precip - dP > -2 * mud_info.weath_unit)
	{
	  char *echo_strings[4] = {
	    "The clouds disappear.",
	    "The clouds disappear.",
	    "The sky begins to break through " "the clouds.",
	    "The clouds are slowly " "evaporating."
	  };

	  replace_str (&weath->echo, echo_strings[n]);
	  replace_str (&weath->echo_color, "{W");
	}
      break;

    case 1:
      if (precip - dP <= -2 * mud_info.weath_unit)
	{
	  char *echo_strings[4] = {
	    "The sky is getting cloudy.",
	    "The sky is getting cloudy.",
	    "Light clouds cast a haze over " "the sky.",
	    "Billows of clouds spread through " "the sky."
	  };

	  replace_str (&weath->echo, echo_strings[n]);
	  replace_str (&weath->echo_color, "{D");
	}
      break;

    case 2:
      if (precip - dP > 0)
	{
	  if (tindex > 1)
	    {
	      char *echo_strings[4] = {
		"The rain stops.",
		"The rain stops.",
		"The rainstorm tapers " "off.",
		"The rain's intensity " "breaks."
	      };

	      replace_str (&weath->echo, echo_strings[n]);
	      replace_str (&weath->echo_color, "{C");
	    }
	  else
	    {
	      char *echo_strings[4] = {
		"The snow stops.",
		"The snow stops.",
		"The snow showers taper " "off.",
		"The snow flakes disappear " "from the sky."
	      };

	      replace_str (&weath->echo, echo_strings[n]);
	      replace_str (&weath->echo_color, "{W");
	    }
	}
      break;

    case 3:
      if (precip - dP <= 0)
	{
	  if (tindex > 1)
	    {
	      char *echo_strings[4] = {
		"It starts to rain.",
		"It starts to rain.",
		"A droplet of rain falls " "upon you.",
		"The rain begins to " "patter."
	      };

	      replace_str (&weath->echo, echo_strings[n]);
	      replace_str (&weath->echo_color, "{C");
	    }
	  else
	    {
	      char *echo_strings[4] = {
		"It starts to snow.",
		"It starts to snow.",
		"Crystal flakes begin to " "fall from the " "sky.",
		"Snow flakes drift down " "from the clouds."
	      };

	      replace_str (&weath->echo, echo_strings[n]);
	      replace_str (&weath->echo_color, "{W");
	    }
	}
      else if (tindex < 2 && temp - dT > -mud_info.weath_unit)
	{
	  char *echo_strings[4] = {
	    "The temperature drops and the rain " "becomes a light snow.",
	    "The temperature drops and the rain " "becomes a light snow.",
	    "Flurries form as the rain freezes.",
	    "Large snow flakes begin to fall " "with the rain."
	  };

	  replace_str (&weath->echo, echo_strings[n]);
	  replace_str (&weath->echo_color, "{W");
	}
      else if (tindex > 1 && temp - dT <= -mud_info.weath_unit)
	{
	  char *echo_strings[4] = {
	    "The snow flurries are gradually " "replaced by pockets of rain.",
	    "The snow flurries are gradually " "replaced by pockets of rain.",
	    "The falling snow turns to a cold drizzle.",
	    "The snow turns to rain as the air warms."
	  };

	  replace_str (&weath->echo, echo_strings[n]);
	  replace_str (&weath->echo_color, "{C");
	}
      break;

    case 4:
      if (precip - dP > 2 * mud_info.weath_unit)
	{
	  if (tindex > 1)
	    {
	      char *echo_strings[4] = {
		"The lightning has stopped.",
		"The lightning has stopped.",
		"The sky settles, and the " "thunder surrenders.",
		"The lightning bursts fade as " "the storm weakens."
	      };

	      replace_str (&weath->echo, echo_strings[n]);
	      replace_str (&weath->echo_color, "{D");
	    }
	}
      else if (tindex < 2 && temp - dT > -mud_info.weath_unit)
	{
	  char *echo_strings[4] = {
	    "The cold rain turns to snow.",
	    "The cold rain turns to snow.",
	    "Snow flakes begin to fall " "amidst the rain.",
	    "The driving rain begins to freeze."
	  };

	  replace_str (&weath->echo, echo_strings[n]);
	  replace_str (&weath->echo_color, "{W");
	}
      else if (tindex > 1 && temp - dT <= -mud_info.weath_unit)
	{
	  char *echo_strings[4] = {
	    "The snow becomes a freezing rain.",
	    "The snow becomes a freezing rain.",
	    "A cold rain beats down on you " "as the snow begins to melt.",
	    "The snow is slowly replaced by a heavy " "rain."
	  };

	  replace_str (&weath->echo, echo_strings[n]);
	  replace_str (&weath->echo_color, "{C");
	}
      break;

    case 5:
      if (precip - dP <= 2 * mud_info.weath_unit)
	{
	  if (tindex > 1)
	    {
	      char *echo_strings[4] = {
		"Lightning flashes in the " "sky.",
		"Lightning flashes in the " "sky.",
		"A flash of lightning splits " "the sky.",
		"The sky flashes, and the " "ground trembles with " "thunder."
	      };

	      replace_str (&weath->echo, echo_strings[n]);
	      replace_str (&weath->echo_color, "{Y");
	    }
	}
      else if (tindex > 1 && temp - dT <= -mud_info.weath_unit)
	{
	  char *echo_strings[4] = {
	    "The sky rumbles with thunder as " "the snow changes to rain.",
	    "The sky rumbles with thunder as " "the snow changes to rain.",
	    "The falling turns to freezing rain " "amidst flashes of "
	      "lightning.",
	    "The falling snow begins to melt as " "thunder crashes overhead."
	  };

	  replace_str (&weath->echo, echo_strings[n]);
	  replace_str (&weath->echo_color, "{W");
	}
      else if (tindex < 2 && temp - dT > -mud_info.weath_unit)
	{
	  char *echo_strings[4] = {
	    "The lightning stops as the rainstorm "
	      "becomes a blinding " "blizzard.",
	    "The lightning stops as the rainstorm "
	      "becomes a blinding " "blizzard.",
	    "The thunder dies off as the "
	      "pounding rain turns to " "heavy snow.",
	    "The cold rain turns to snow and " "the lightning stops."
	  };

	  replace_str (&weath->echo, echo_strings[n]);
	  replace_str (&weath->echo_color, "{C");
	}
      break;

    default:
      bug ("echo_weather: invalid precip index");
      weath->precip = 0;
      break;
    }

  return;
}


void
get_time_echo (WeatherData * weath)
{
  int n;
  int pindex;

  n = number_bits (2);
  pindex =
    (weath->precip + 3 * mud_info.weath_unit - 1) / mud_info.weath_unit;
  replace_str (&weath->echo, "");
  replace_str (&weath->echo_color, "{w");

  switch (time_info.hour)
    {
    case 5:
      {
	char *echo_strings[4] = {
	  "The day has begun.",
	  "The day has begun.",
	  "The sky slowly begins to glow.",
	  "The sun slowly embarks upon a new day."
	};

	time_info.sunlight = SUN_RISE;
	replace_str (&weath->echo, echo_strings[n]);
	replace_str (&weath->echo_color, "{Y");
	break;
      }
    case 6:
      {
	char *echo_strings[4] = {
	  "The sun rises in the east.",
	  "The sun rises in the east.",
	  "The hazy sun rises over the horizon.",
	  "Day breaks as the sun lifts into the sky."
	};

	time_info.sunlight = SUN_LIGHT;
	replace_str (&weath->echo, echo_strings[n]);
	replace_str (&weath->echo_color, "{y");
	break;
      }
    case 12:
      {
	if (pindex > 0)
	  {
	    replace_str (&weath->echo, "It's noon.");
	  }
	else
	  {
	    char *echo_strings[2] = {
	      "The intensity of the sun " "heralds the noon hour.",
	      "The sun's bright rays beat down " "upon your shoulders."
	    };

	    replace_str (&weath->echo, echo_strings[n % 2]);
	  }
	time_info.sunlight = SUN_LIGHT;
	replace_str (&weath->echo_color, "{W");
	break;
      }
    case 19:
      {
	char *echo_strings[4] = {
	  "The sun slowly disappears in the west.",
	  "The reddish sun sets past the horizon.",
	  "The sky turns a reddish orange as the sun " "ends its journey.",
	  "The sun's radiance dims as it sinks in the " "sky."
	};

	time_info.sunlight = SUN_SET;
	replace_str (&weath->echo, echo_strings[n]);
	replace_str (&weath->echo_color, "{R");
	break;
      }
    case 20:
      {
	if (pindex > 0)
	  {
	    char *echo_strings[2] = {
	      "The night begins.",
	      "Twilight descends around you."
	    };

	    replace_str (&weath->echo, echo_strings[n % 2]);
	  }
	else
	  {
	    char *echo_strings[2] = {
	      "The moon's gentle glow diffuses " "through the night sky.",
	      "The night sky gleams with " "glittering starlight."
	    };

	    replace_str (&weath->echo, echo_strings[n % 2]);
	  }
	time_info.sunlight = SUN_DARK;
	replace_str (&weath->echo_color, "{b");
	break;
      }
    }

  return;
}

Olc_Fun (aedit_climate)
{
  char arg[MAX_INPUT_LENGTH];
  AreaData *area;

  GetEdit (ch, AreaData, area);

  argument = one_argument (argument, arg);


  if (NullStr (arg))
    {
      chprintlnf (ch, "%s:", area->name);
      chprintlnf (ch, "\tTemperature:\t%s",
		  temp_settings[area->weather.climate_temp]);
      chprintlnf (ch, "\tPrecipitation:\t%s",
		  precip_settings[area->weather.climate_precip]);
      chprintlnf (ch, "\tWind:\t\t%s",
		  wind_settings[area->weather.climate_wind]);

      chprintlnf (ch, "Weather variables range from "
		  "%d to %d.", -3 * mud_info.weath_unit,
		  3 * mud_info.weath_unit);
      chprintlnf (ch, "Weather vectors range from "
		  "%d to %d.", -1 * mud_info.max_vector, mud_info.max_vector);
      chprintlnf (ch,
		  "The maximum a vector can " "change in one update is %d.",
		  mud_info.rand_factor + 2 * mud_info.climate_factor +
		  3 * mud_info.weath_unit);

      cmd_syntax (ch, NULL, n_fun, "temp <number>", "precip <number>",
		  "wind <number>", NULL);

      return false;
    }

  else if (!str_cmp (arg, "temp"))
    {
      int i;

      argument = one_argument (argument, arg);

      for (i = 0; i < MAX_CLIMATE; i++)
	{
	  if (str_cmp (arg, temp_settings[i]))
	    continue;

	  area->weather.climate_temp = i;
	  olc_msg (ch, n_fun, "The climate temperature "
		   "for %s is now %s.", area->name, temp_settings[i]);
	  return true;
	}

      if (i == MAX_CLIMATE)
	{
	  chprintlnf (ch, "Possible temperature settings:");
	  for (i = 0; i < MAX_CLIMATE; i++)
	    {
	      chprintlnf (ch, "\t%s", temp_settings[i]);
	    }
	}

      return false;
    }

  else if (!str_cmp (arg, "precip"))
    {
      int i;

      argument = one_argument (argument, arg);

      for (i = 0; i < MAX_CLIMATE; i++)
	{
	  if (str_cmp (arg, precip_settings[i]))
	    continue;

	  area->weather.climate_precip = i;
	  olc_msg (ch, n_fun, "The climate precipitation "
		   "for %s is now %s.", area->name, precip_settings[i]);
	  return true;
	}

      if (i == MAX_CLIMATE)
	{
	  chprintlnf (ch, "Possible precipitation settings:");
	  for (i = 0; i < MAX_CLIMATE; i++)
	    {
	      chprintlnf (ch, "\t%s", precip_settings[i]);
	    }
	}

      return false;
    }

  else if (!str_cmp (arg, "wind"))
    {
      int i;

      argument = one_argument (argument, arg);

      for (i = 0; i < MAX_CLIMATE; i++)
	{
	  if (str_cmp (arg, wind_settings[i]))
	    continue;

	  area->weather.climate_wind = i;
	  olc_msg (ch, n_fun, "The climate wind for %s "
		   "is now %s.", area->name, wind_settings[i]);
	  return true;
	}

      if (i == MAX_CLIMATE)
	{
	  chprintlnf (ch, "Possible wind settings:");
	  for (i = 0; i < MAX_CLIMATE; i++)
	    {
	      chprintlnf (ch, "\t%s", wind_settings[i]);
	    }
	}

      return false;
    }
  else
    {
      cmd_syntax (ch, NULL, n_fun, "temp <number>", "precip <number>",
		  "wind <number>", NULL);

      return false;
    }
}


void
time_update ()
{
  AreaData *pArea;
  Descriptor *d;
  WeatherData *weath;

  switch (++time_info.hour)
    {
    case 5:
    case 6:
    case HOURS_IN_DAY / 2:
    case 19:
    case 20:
      for (pArea = area_first; pArea; pArea = pArea->next)
	{
	  get_time_echo (&pArea->weather);
	}

      for (d = descriptor_first; d; d = d->next)
	{
	  if (d->connected == CON_PLAYING &&
	      IsOutside (d->character) && IsAwake (d->character))
	    {
	      weath = &d->character->in_room->area->weather;
	      if (NullStr (weath->echo))
		continue;
	      d_printlnf (d, "%s%s{x", weath->echo_color, weath->echo);
	    }
	}
      break;
    case HOURS_IN_DAY:
      time_info.hour = 0;
      time_info.day++;
      if (time_info.day % 10 == 0)
	rw_time_data (act_write);
      break;
    }

  if (time_info.day >= DAYS_IN_MONTH)
    {
      time_info.day = 0;
      time_info.month++;
    }

  if (time_info.month >= MONTHS_IN_YEAR)
    {
      time_info.month = 0;
      time_info.year++;
    }

  return;
}
