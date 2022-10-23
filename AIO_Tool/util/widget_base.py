# -*- coding: utf-8 -*-
################################################################################
#
# Author: ClimbSnail(HQ)
# original source is here.
#   https://github.com/ClimbSnail/HoloCubic_AIO_Tool
# 
#
################################################################################

import tkinter as tk


class EntryWithPlaceholder(tk.Entry):
    def __init__(self, master=None, *, placeholder="PLACEHOLDER",
                 placeholder_color='grey', **attribute):
        super().__init__(master, attribute)

        self.placeholder = placeholder.strip()
        self.placeholder_color = placeholder_color
        self.default_fg_color = self['fg']

        self.bind("<FocusIn>", self.foc_in)
        self.bind("<FocusOut>", self.foc_out)

    def get(self):
        if super().get().strip() == self.placeholder:
            return ""
        return super().get().strip()

    def refresh(self):
        """
        若输入框带有其他默认值（非提示词）时 需要手动调用刷新
        """
        if not self.get():
            self.insert(0, self.placeholder)
            self['fg'] = self.placeholder_color

    def foc_in(self, *args):
        if self.placeholder == super().get():
            if self['fg'] == self.placeholder_color:
                self.delete('0', 'end')
                self['fg'] = self.default_fg_color

    def foc_out(self, *args):
        self.refresh()
