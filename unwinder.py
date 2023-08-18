from gdb.unwinder import Unwinder

g_cthread_main_address = None

def get_main_address():
	global g_cthread_main_address
	if g_cthread_main_address == None:
		disasm = gdb.execute("disassemble __morestack", False, True)
		disasm = disasm.splitlines()[1:-1]
		disasm = [int(l.lstrip().split()[0], 16) for l in disasm]
		g_cthread_main_address = disasm
	return g_cthread_main_address

class FrameID:
    def __init__(self, sp, pc):
        self.sp = sp
        self.pc = pc

class cthread_unwinder(Unwinder):
	def __init__(self):
		super().__init__("cthread_unwinder")

	def __call__(self, pending_frame):
		try:
			# Analyze the function we're going to unwind.
			addr = get_main_address()
			pc = pending_frame.read_register("pc")
			# print(f"test {pc}")
			if pc < addr[0] or pc > addr[-1]:
				return None
			rsp = pending_frame.read_register("rsp")
			rbp = pending_frame.read_register("rbp")
			# print(f"Found a frame we can handle at: {pc} {rsp} {rbp}")
			prev_rsp = gdb.parse_and_eval("*((void**) 0x%x)" % (rbp - 8))
			prev_rbp = gdb.parse_and_eval("*((void**) 0x%x)" % (rbp))
			prev_pc = gdb.parse_and_eval("*((void**) 0x%x)" % (rbp + 8))
			# print(f"Previous at: {prev_pc} {prev_rsp} {prev_rbp}")

			frame_id = FrameID(prev_rsp, prev_pc)
			unwind_info = pending_frame.create_unwind_info(frame_id)
			unwind_info.add_saved_register("rsp", prev_rsp)
			unwind_info.add_saved_register("rbp", prev_rbp)
			unwind_info.add_saved_register("pc", prev_pc)
			# print(unwind_info)
			return unwind_info
		except Exception as e:
			print(e)
			return None

gdb.unwinder.register_unwinder(None, cthread_unwinder(), replace=True)
