import pandas as pd
import numpy as np
import zipfile
import wget
import plotly
import plotly.graph_objs as go
import plotly.express as px
from plotly.subplots import make_subplots
from dateutil.parser import parse
import datetime
from dash import Dash, html, dcc, Input, Output
import plotly.express as px
import json
import plotly.express as px
from urllib.request import urlopen

url = 'https://github.com/Palladain/Deep_Python/raw/main/Homeworks/Homework_1/archive.zip'
filename = wget.download(url)

with zipfile.ZipFile(filename, 'r') as zip_ref:
    zip_ref.extractall('./')

customers = pd.read_csv('olist_customers_dataset.csv')
location = pd.read_csv('olist_geolocation_dataset.csv')
items = pd.read_csv('olist_order_items_dataset.csv')
payments = pd.read_csv('olist_order_payments_dataset.csv')
reviews = pd.read_csv('olist_order_reviews_dataset.csv')
orders = pd.read_csv('olist_orders_dataset.csv')
products = pd.read_csv('olist_products_dataset.csv')
translation = pd.read_csv('product_category_name_translation.csv')
sellers = pd.read_csv('olist_sellers_dataset.csv')
pd.options.mode.chained_assignment = None

tab4=pd.merge(items, orders, how="inner", on="order_id")
tab4=pd.merge(tab4, sellers, how="inner", on="seller_id")
tab4=pd.merge(tab4, customers, how="inner", on="customer_id")
tab4=pd.merge(tab4, products, how="inner", on="product_id")
tab4=pd.merge(tab4, translation, how="inner", on="product_category_name")

tab4["order_purchase_timestamp"]=tab4["order_purchase_timestamp"].apply(lambda x: parse(x).date())

dates=tab4.drop(columns=tab4.columns.difference(["order_purchase_timestamp"])).sort_values(by="order_purchase_timestamp").drop_duplicates().reset_index()
dates_num=np.arange(1, len(dates)+1)
dates["dates_num"]=dates_num

tab4=pd.merge(tab4, dates, how="inner", on="order_purchase_timestamp")
tab4=tab4.drop(columns=tab4.columns.difference(["order_id", "product_category_name_english", "order_purchase_timestamp", "dates_num", "order_status", "seller_state", "customer_state"]))

tab4["product_category_name_english"]=tab4["product_category_name_english"].apply(lambda x: x.title().replace("_", " "))
states=list(customers["customer_state"].unique())
states.append("Brazil")



external_stylesheets = ['https://codepen.io/achriddyp/pen/bWLwgP.css']
app = Dash(__name__, external_stylesheets=external_stylesheets)
df = pd.read_csv('https://plotly.github.io/datasets/country_indicators.csv')

app.layout = html.Div([
    html.Div([dcc.Dropdown(options=states, value='Brazil', id='crossfilter-state')],
             style={"width": "25%", 'display': 'inline-block'}),

    html.Div([dcc.Dropdown(options=list(tab4['order_status'].unique()), value=list(['delivered', 'canceled']),
                           id='crossfilter-status', multi=True)], style={"width": "70%", 'display': 'inline-block'}),

    html.Div(dcc.Slider(min=1, max=616, step=1, value=10, id='crossfilter-date-slider', marks=None),
             style={'width': '100%'}),

    html.Div([dcc.Graph(id='crossfilter-barchart')], style={'width': '100%', 'display': 'inline-block'})])


@app.callback(
    Output('crossfilter-barchart', 'figure'),
    Input('crossfilter-state', 'value'),
    Input('crossfilter-status', 'value'),
    Input('crossfilter-date-slider', 'value'))
def update_graph(state, statuses, date):
    statuses = list(statuses)
    stat_line = ""
    for st in statuses:
        stat_line += st + ', '
    stat_line = stat_line[:-2]

    tab4_sell = tab4[tab4["dates_num"] == date]
    if state != "Brazil":
        tab4_sell = tab4_sell[tab4_sell["seller_state"] == state]

    tab4_sell["flag"] = tab4_sell["order_status"].apply(lambda x: x in statuses)
    tab4_sell = tab4_sell[tab4_sell["flag"] == True]
    tab4_sell = tab4_sell.groupby(["product_category_name_english", "order_status"]).agg(
        {"order_id": "count"}).reset_index()

    tab4_cust = tab4[tab4["dates_num"] == date]
    if state != "Brazil":
        tab4_cust = tab4_cust[tab4_cust["customer_state"] == state]

    tab4_cust["flag"] = tab4_cust["order_status"].apply(lambda x: x in statuses)
    tab4_cust = tab4_cust[tab4_cust["flag"] == True]
    tab4_cust = tab4_cust.groupby(["product_category_name_english", "order_status"]).agg(
        {"order_id": "count"}).reset_index()

    fig = make_subplots(rows=2, cols=1, vertical_spacing=0.07,
                        subplot_titles=[
                            f"Categories of {stat_line} goods sold in {state} on {dates[dates['dates_num'] == date]['order_purchase_timestamp'].values[0]}",
                            f"Categories of {stat_line} goods bought in {state} on {dates[dates['dates_num'] == date]['order_purchase_timestamp'].values[0]}"])

    for curr_status in statuses:
        fig.add_trace(go.Bar(y=tab4_sell[tab4_sell["order_status"] == curr_status]["product_category_name_english"],
                             x=tab4_sell[tab4_sell["order_status"] == curr_status]["order_id"], width=0.5,
                             orientation='h', name=curr_status,
                             marker_color=tab4_sell[tab4_sell["order_status"] == curr_status]["order_id"]), 1, 1)

        fig.add_trace(go.Bar(y=tab4_cust[tab4_cust["order_status"] == curr_status]["product_category_name_english"],
                             x=tab4_cust[tab4_cust["order_status"] == curr_status]["order_id"], width=0.5,
                             orientation='h', name=curr_status,
                             marker_color=tab4_cust[tab4_cust["order_status"] == curr_status]["order_id"]), 2, 1)

    fig.update_layout(height=1200, width=1000, showlegend=False, title=dict(x=0.5, xanchor="center"), barmode="stack",
                      xaxis1=dict(categoryorder="total descending", tickmode='linear', tick0=0, dtick=1),
                      xaxis2=dict(categoryorder="total descending", tickmode='linear', tick0=0, dtick=1))

    fig.update_xaxes(title='Number of sales', col=1, row=1)
    fig.update_xaxes(title='Number of purchases', col=1, row=2)
    fig.update_yaxes(title='Categories', col=1, row=1)
    fig.update_yaxes(title='Categories', col=1, row=2)
    fig.update_traces(hoverinfo="all", hovertemplate="Category: %{y}<br>Quantity: %{x}")

    return fig


if __name__ == '__main__':
    app.run_server(debug=True)