import scrapy
import requests
from bs4 import BeautifulSoup
from lxml import etree
from .. import items
from urllib.parse import urlencode
from urllib.parse import urljoin


class SteamspiderSpider(scrapy.Spider):
    name = "SteamSpider"
    allowed_domains = ["store.steampowered.com"]
    my_categories=["Экшен", "Инди", "Приключения"]
    my_pages=["1", "2"]
    start_urls = []
    for cat in my_categories:
      for p in my_pages:
        curr_url = 'https://store.steampowered.com/search/?' + urlencode({'term': cat, 'ignore_preferences': '1', 'ndl':'1', 'page': p})
        start_urls.append(curr_url)


    def start_requests(self):
        for curr_url in self.start_urls:
            yield scrapy.Request(curr_url, callback=self.parse)

    def parse(self, response):
        game_items = items.SpiderSteamItem()
        root = BeautifulSoup(response.body, 'html.parser')
        dom_root=etree.HTML(str(root))

        game_page_links=dom_root.xpath('//a[@class="search_result_row ds_collapse_flag"]')

        for i in range(len(game_page_links)):
          game_link=game_page_links[i].get("href")

          game_req = requests.get(game_link)
          page = game_req.content.decode("utf-8")
          game = BeautifulSoup(page, 'html.parser')
          dom_game = etree.HTML(str(game))

          release_date=' '.join(dom_game.xpath('//div[@class="app_header_grid_container"]/div[@class="grid_content grid_date"]/text()')[0].split())
          if (release_date[-4:]<'2000'):
            continue

          name=dom_game.xpath('//div[@class="apphub_AppName"]/text()')[0]
          category=dom_game.xpath('//div[@class="blockbg"]/a/text()')[1:]

          reviews_number=""
          overall_grade=""

          marker=dom_game.xpath('//div[@class="user_reviews_summary_row"]')[0].get("data-tooltip-html")
          if (marker=="No user reviews"):
            reviews_number=0
            overall_grade="No user reviews, no grade yet"
          else:
            info=list(map(lambda x: x.strip(), dom_game.xpath('//div[@class="summary column"]/span/text()')))
            if (len(info)>6):
              reviews_number=info[4][1:-1]
              overall_grade=info[3]
            else:
              reviews_number=info[1][1:-1]
              overall_grade=info[0]
            if (reviews_number[0]==" "):
              reviews_number=overall_grade[0]
              overall_grade="Need more user reviews to generate a score"

          developer=dom_game.xpath('//div[@class="app_header_grid_container"]//a/text()')[0]
          tags=list(map(lambda x: x.strip(), dom_game.xpath('//a[@class="app_tag"]/text()')))
          
          price_res=""
          sample_free=root.find_all(class_="col search_price_discount_combined responsive_secondrow")[i].find(class_="discount_final_price free")
          sample_discount=root.find_all(class_="col search_price_discount_combined responsive_secondrow")[i].find(class_="discount_final_price")
          if (sample_free!=None):
            price_res=sample_free.text
          elif (sample_discount!=None):
            price_res=sample_discount.text
          else:  
            price_res="No price yet, game has not been released"

          platforms_raw=dom_game.xpath('//div[@class="sysreq_contents"]/div')
          platforms=[]
          for j in range(len(platforms_raw)):
            platforms.append(platforms_raw[j].get("data-os"))

          game_items['name'] = name
          game_items["category"]=category
          game_items["reviews_number"]=reviews_number
          game_items["overall_grade"]=overall_grade
          game_items["release_date"]=release_date
          game_items["developer"]=developer
          game_items["tags"]=tags
          game_items["price"]=price_res
          game_items["platforms"]=platforms
          yield game_items