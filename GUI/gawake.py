import gi
gi.require_version('Gtk', '3.0')

from gi.repository import Gtk

builder = Gtk.Builder()

builder.add_from_file('user_interface.glade')

class Handler(object):
    def __init__(self, **kwargs):
        super(Handler, self).__init__(**kwargs)

        self.lb_text = builder.get_object('lb_text')
        self.lb_text.set_text('Testando Gawake...')

    def on_main_window_destroy(self, window):
        Gtk.main_quit()

builder.connect_signals(Handler())
window = builder.get_object('main_window')
window.show_all()

if __name__ == '__main__':
    print('Exiting')
    Gtk.main()
