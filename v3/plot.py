import plotly.graph_objects as go
import sys

if len(sys.argv) != 3:
    print("Usage: python3 plot.py <0/1> <filename>")
    sys.exit(1)

if sys.argv[2] == '0':
    name = "Throughput"
elif sys.argv[2] == '2':
    name = "CPU Utilization"
elif sys.argv[2] == '1':
    name = "Average Response Time"
else:
    print("Unkown")
    exit(1)

#read data from file
with open(sys.argv[1]) as f:
    lines = f.readlines()

#remove whitespace characters like `\n` at the end of each line
lines = [x.strip() for x in lines]

x_values = []
y_values = []

for line in lines:
    columns = line.split()
    if len(columns) >= 2:
        x_values.append(float(columns[0]))
        y_value = float(columns[1])
        y_values.append(y_value)


# Create traces
trace = go.Scatter(x=x_values,y=y_values,mode='lines+markers',name=name)

#edit layout
layout = go.Layout(title=name,xaxis=dict(title='Clients'),yaxis=dict(title=name))

fig = go.Figure(data=trace,layout=layout)

# fig.show()

output_image_path = f'{name}_plot.png'
fig.write_image(output_image_path)

# Print a message with the saved file path
print(f"Plot saved as: {output_image_path}")
