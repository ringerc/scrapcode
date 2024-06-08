#!/usr/bin/env python3

"""
Scrapy spider to find links to ands.org.au and its subdomains
from a list of target sites

Writes to links.csv in same dir as crawler.

Run with 'scrapy runspider ands_spider.py'

I didn't bother adding command line args for the sites to crawl or the allowed
domains, just hardcode them. If you want to add command line args, see
https://docs.scrapy.org/en/latest/topics/spiders.html#spider-arguments

The matcher for links is very dumb. I haven't added any de-duplication
of output etc.
"""

import scrapy
import pathlib


class LinkItem(scrapy.item.Item):
    page_url = scrapy.item.Field()
    link_url = scrapy.item.Field()
    link_text = scrapy.item.Field()

class AndsSpider(scrapy.Spider):
    """
    A Scrapy spider to look for links to ands.org.au and report them.

    See https://docs.scrapy.org/en/latest/topics/spiders.html
    """

    name = 'ands'

    start_urls = [
            'https://research.unsw.edu.au',
            ]

    # Use offsite middleware to filter crawl
    # https://docs.scrapy.org/en/latest/topics/spider-middleware.html#scrapy.spidermiddlewares.offsite.OffsiteMiddleware
    allowed_domains = [
            'research.unsw.edu.au',
            ]

    # scrapy settings
    # you can pass these as -s KEY=VALUE on cmdline instead
    custom_settings = {
            # Save output
            # https://docs.scrapy.org/en/latest/topics/feed-exports.html
            'FEEDS': {
                pathlib.Path('links.csv'): {
                    'format': 'csv',
                    'fields': ['page_url', 'link_url', 'link_text'],
                    },
                },

            # Limit crawl rate
            # https://docs.scrapy.org/en/latest/topics/autothrottle.html
            'AUTOTHROTTLE_ENABLED': True,

            # Resumeable, save progress
            # https://docs.scrapy.org/en/latest/topics/jobs.html
            'JOBDIR': pathlib.Path('crawl_status')
            }

    def __init__(self):
        self.link_extractor = scrapy.linkextractors.lxmlhtml.LxmlLinkExtractor()
        #self.feed_options = feed_options
        self.exporter = scrapy.exporters.CsvItemExporter

    def parse(self, response):
        for link in self.link_extractor.extract_links(response):
            if 'ands.org.au' in link.url:
                yield LinkItem(page_url = response.url, link_url = link.url, link_text= link.text)
            # allowed_domains will filter the links
            yield response.follow(link, self.parse)


# vim: ts=4 et sw=4 ai
