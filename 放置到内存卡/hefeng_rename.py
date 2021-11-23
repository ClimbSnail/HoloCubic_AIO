import os
path = "weather_hefeng/"
for file in os.listdir(path): 
    name = file.split('_')
    if len(name) == 2:
        print("256/"+name[1], "256/"+file)
        os.rename("256/"+name[1], "256/"+file)