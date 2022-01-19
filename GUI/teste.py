# import GTK module to access classes and functions from GTK+
import gi

# require version 3 of GTK
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk

# creating a class for the window
class MyWindow(Gtk.Window):
    def __init__(self):
        # calling constructor from the superclass
        super().__init__(title="Gawake")

        self.button = Gtk.Button(Gtk.Label(label="Add rule", angle=25, halign=Gtk.Align.END))
        self.button.connect("clicked", self.on_button_clicked)
        self.add(self.button)

    def on_button_clicked(self, widget):
        print("Add rule function")    

win = MyWindow()

# when click on "X" button, the window will close
win.connect("destroy", Gtk.main_quit)

# show the window
win.show_all()

Gtk.main()

# handler_id = widget.connect("event", callback/function, data)
