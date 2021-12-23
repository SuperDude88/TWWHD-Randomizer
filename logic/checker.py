import json

with open('GameItem.hpp') as game_item_file:
    game_item_data = game_item_file.read()
game_item_data = game_item_data.partition('enum struct GameItem')[2]
game_item_data = game_item_data.partition('{\n')[2]
items = game_item_data.partition('\n}')[0]
items = items.split('\n')
items = {item.replace(',', '').strip() for item in items}

with open('Location.hpp') as location_file:
    location_data = location_file.read()
location_data = location_data.partition('enum struct Location')[2]
location_data = location_data.partition('{\n')[2]
location_data = location_data.partition('\n}')[0]
locations = {l.replace(',', '').strip() for l in location_data.split('\n')}

with open('Setting.hpp') as setting_file:
    setting_data = setting_file.read()
setting_data = setting_data.partition('enum struct Setting')[2]
setting_data = setting_data.partition('{\n')[2]
setting_data = setting_data.partition('\n}')[0]
settings = {set.replace(',', '').strip() for set in setting_data.split('\n')}

valid = locations | items | settings

with open("Macros.json") as macros_file:
    macros = json.load(macros_file)['Macros']

encounterd_macros = set()

for macro in macros:
    encounterd_macros.add(macro['Name'])

def check_requirement(req):
    args = req['args']  # type: List
    while len(args) > 0:
        arg = args.pop()
        if isinstance(arg, dict):
            args += arg['args']
        elif isinstance(arg, (int, float)):
            continue
        elif isinstance(arg, str):
            if arg not in valid and arg not in encounterd_macros:
                print(f'Encounterd Invalid Str Arg: {arg}')
        else:
            print('???')

def check_macros(elts):
    for elt in elts:
        name = elt['Name']
        expr = elt['Expression']
        check_requirement(expr)
        

def check_locations(locs):
    for loc in locs:
        check_requirement(loc["Needs"])

with open("locations.json") as location_file:
    locations_j = json.load(location_file)["Locations"]

check_macros(macros)
check_locations(locations_j)

