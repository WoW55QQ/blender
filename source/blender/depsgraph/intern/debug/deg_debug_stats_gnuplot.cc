/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2017 Blender Foundation.
 * All rights reserved.
 *
 * Original Author: Sergey Sharybin
 * Contributor(s): None Yet
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/depsgraph/intern/debug/deg_debug_stats_gnuplot.cc
 *  \ingroup depsgraph
 */

#include "DEG_depsgraph_debug.h"

#include <stdarg.h>

#include "BLI_compiler_attrs.h"

#include "intern/depsgraph.h"
#include "intern/nodes/deg_node_id.h"

#include "util/deg_util_foreach.h"

extern "C" {
#include "DNA_ID.h"
} /* extern "C" */

#define NL "\r\n"

namespace DEG {
namespace {

struct DebugContext {
	FILE *file;
	const Depsgraph *graph;
	const char *label;
	const char *output_filename;
};

/* TODO(sergey): De-duplicate with graphviz relation debugger. */
static void deg_debug_fprintf(const DebugContext &ctx,
                              const char *fmt,
                              ...) ATTR_PRINTF_FORMAT(2, 3);
static void deg_debug_fprintf(const DebugContext &ctx, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(ctx.file, fmt, args);
	va_end(args);
}

void write_stats_data(const DebugContext& ctx)
{
	deg_debug_fprintf(ctx, "$data << EOD" NL);
	// TODO(sergey): Sort nodes by time.
	foreach (const IDDepsNode *id_node, ctx.graph->id_nodes) {
		// TODO(sergey): Figure out a nice way to define which exact time
		// we want to show.
		const double time = id_node->stats.current_time;
		if (time == 0.0) {
			continue;
		}
		deg_debug_fprintf(ctx, "\"%s\",%f" NL,
		                  id_node->id_orig->name + 2,
		                  time);
	}
	deg_debug_fprintf(ctx, "EOD" NL);
}

void deg_debug_stats_gnuplot(const DebugContext& ctx)
{
	// Data itself.
	write_stats_data(ctx);
	// Optional label.
	if (ctx.label && ctx.label[0]) {
		deg_debug_fprintf(ctx, "set title \"%s\"" NL, ctx.label);
	}
	// Rest of the commands.
	// TODO(sergey): Need to decide on the resolution somehow.
	deg_debug_fprintf(ctx, "set terminal pngcairo size 1920,1080" NL);
	deg_debug_fprintf(ctx, "set output \"%s\"" NL, ctx.output_filename);
	deg_debug_fprintf(ctx, "set grid" NL);
	deg_debug_fprintf(ctx, "set datafile separator ','" NL);
	deg_debug_fprintf(ctx, "set style fill solid" NL);
	deg_debug_fprintf(ctx, "plot \"$data\" using " \
	                       "($2*0.5):0:($2*0.5):(0.2):yticlabels(1) "
	                       "with boxxyerrorbars t '' lt rgb \"#406090\"" NL);

}

}  // namespace
}  // namespace DEG

void DEG_debug_stats_gnuplot(const Depsgraph *depsgraph,
                             FILE *f,
                             const char *label,
                             const char *output_filename)
{
	if (depsgraph == NULL) {
		return;
	}
	DEG::DebugContext ctx;
	ctx.file = f;
	ctx.graph = (DEG::Depsgraph *)depsgraph;
	ctx.label = label;
	ctx.output_filename = output_filename;
	DEG::deg_debug_stats_gnuplot(ctx);
}
