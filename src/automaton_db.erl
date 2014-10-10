-module(automaton_db).

-export([open/1, get/3, q/2, put/2]).

open(Database)->
    erlflux_http:create_database(Database).

get(Database, FieldNames, SeriesName)->
    erlflux_http:read_point(Database, FieldNames, SeriesName).

q(Database, Query)->
    erlflux_http:q(Database, Query).

put(Database, SeriesData)->
    erlflux_http:write_series(Database, SeriesData).