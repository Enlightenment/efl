focus_widget_tree_bench = executable('focus_widget_tree_bench',
  'focus_widget_tree.c',
  dependencies: [elementary, ecore_input_evas],
)

benchmark('focus_widget_tree', focus_widget_tree_bench,
  args: ['5'],
)

item_container = executable('item_container',
  'item_container.c',
  dependencies: [elementary, ecore_input_evas, eio],
)

benchmark('item_container', item_container)
