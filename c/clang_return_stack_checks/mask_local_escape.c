/*
 * Demonstrate how to trick llvm scan-build's core.StackAddressEscape checker
 * via simple indirection.
 */

#include <stdio.h>
#include <assert.h>

struct chain_container {
	int * leaky_member;
	struct chain_container * previous;
};

struct chain_container * sneaky_chain_container = 0;

void
escape_via_sneaky_chain_assignment_1(void)
{
	struct chain_container new_entry = {0,0};
	int g = 1;
	fprintf(stderr, "&g = %p\n", &g);
	/* g escapes into global scope */
	sneaky_chain_container->leaky_member = &g;
	/* Pointer to container is replaced with indirect pointer via chain */
	new_entry.previous = sneaky_chain_container;
	sneaky_chain_container = &new_entry;

	/*
	 * Restore container pointer from caller.
	 *
	 * scan-build has lost track of &g in its leaky_member now.
	 */
	sneaky_chain_container = sneaky_chain_container->previous;

	/* even though we definitely leak it */
	assert(sneaky_chain_container->leaky_member == &g);
}

void
escape_via_sneaky_chain_assignment(void)
{
	struct chain_container parent_sneaky = {0,0};
	sneaky_chain_container = &parent_sneaky;
	escape_via_sneaky_chain_assignment_1();
	/*
	 * pointer to auto variable from escape_via_sneaky_chain_assignment_1
	 * has escaped. Dereferencing it would be a memory error.
	 */
	fprintf(stderr, "&g has escaped: %p\n",
			sneaky_chain_container->leaky_member);

	/*
	 * Ensure we don't complain about &parent_sneaky escaping.
	 */
	sneaky_chain_container = 0;
}

int
main(void)
{
	escape_via_sneaky_chain_assignment();
	return 0;
}
