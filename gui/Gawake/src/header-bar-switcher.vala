namespace Gawake {
    public class StackSwitcher : Gtk.StackSwitcher {
       public Gtk.StackSwitcher stack_switcher = new Gtk.StackSwitcher();

       construct {
            var stack = new Gtk.Stack();
            stack_switcher.set_stack(stack);

            var menu_button = new Gtk.MenuButton();

            // needs attention
            var page1 = new Gtk.Box(Gtk.Orientation.VERTICAL, 5);
            page1.append(new Gtk.Label("Page 1"));
            var page2 = new Gtk.Box(Gtk.Orientation.VERTICAL, 5);
            page2.append(new Gtk.Label("Page 2"));

            stack.add_titled(page1, "page1", "Page 1");
            stack.add_titled(page2, "page2",  "Page 2");

            headerbar.set_title_widget(stack_switcher);
            headerbar.pack_end(menu_button);
       }

       public Gtk.HeaderBar get_headerbar() {
            return this.headerbar_switcher;
       }
   }
}
