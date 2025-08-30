#include "std_include.hpp"

namespace components
{
	namespace cmd
	{
		bool show_api_lights = false;
		bool show_mesh_bone_info_attached = false;
		bool show_mesh_bone_info = false;
	}

	/**
	 * Initializes the light interpolator
	 * @param l					Parent light
	 * @param points			Reference to point-list
	 * @param looping			Light is looping
	 * @param loop_smoothing	Add additional segment between last and first point
	 * @return
	 */
	bool remix_lights::light::interpolator::init(const std::vector<map_settings::remix_light_settings_s::point_s>& points, const bool looping, const bool loop_smoothing)
	{
		if (points.size() == 1) {
			return false;
		}

		m_initialized = true;
		m_points = points;
		m_looping = looping;
		m_loop_smoothing = loop_smoothing;

		// ensure first point has timepoint 0
		(m_points)[0].timepoint = 0.0f;

		// total duration defined by the last point
		m_total_duration = m_points.back().timepoint;

		if (points.size() > 1 && m_total_duration == 0.0f)
		{
			game::console();
			std::cout << "[RemixLights][light_interpolator::init] Encountered a light were the last point has no defined timepoint! Placeholder in-use, please fix!" << std::endl;

			// use timepoint of prev. point + 1.0
			m_total_duration = (m_points)[m_points.size() - 2].timepoint + 1.0f;

			// write the placeholder value into the last point
			m_points.back().timepoint = m_total_duration;
		}

		// calculate time for points with no defined timepoint
		bool needs_timepoint_calc = false;
		size_t calc_index_start = 0;

		for (size_t i = 1; i < m_points.size(); ++i)
		{
			// point has no defined timepoint
			if ((m_points)[i].timepoint == 0.0f)
			{
				needs_timepoint_calc = true;
				if (calc_index_start == 0) {
					calc_index_start = i;
				}
			}
			else // point with timepoint
			{
				if (needs_timepoint_calc) // check if previous points had no timepoint
				{
					interpolate_timepoints(calc_index_start, i); // evenly distribute time
					needs_timepoint_calc = false;
					calc_index_start = 0;
				}
			}
		}

		calculate_segment_durations();
		return true;
	}

	/**
	 * Advances time
	 * @param frametime		Time of the last frame
	 * @return				True if move is done (also true for looping lights)
	 */
	bool remix_lights::light::interpolator::advance_time(const float frametime)
	{
		m_elapsed_time += frametime;

		if (m_elapsed_time >= m_total_duration)
		{
			if (m_looping) {
				m_elapsed_time -= m_total_duration;
			}
			else {
				m_elapsed_time = m_total_duration;
			}

			return true;
		}

		return false;
	}

	Vector remix_lights::light::calculate_direction_for_point(const map_settings::remix_light_settings_s::point_s* point) const
	{
		if (is_attached())
		{
			// get direction vector for angles
			Vector offset_dir;
			utils::vector::AngleVectors(point->angle_offset_attached, &offset_dir);

			// apply offset in bone's local coordinate system
			Vector world_offset = offset_dir.x * m_attached_bone_forward + offset_dir.y * m_attached_bone_right + offset_dir.z * m_attached_bone_up;

			// combine with bone's forward (scale to achieve full offset)
			//float offset_scale = std::numbers::sqrt2_v<float>; // ca. 90deg max offset
			//Vector final_dir = (m_attached_bone_forward + offset_scale * world_offset);
			//final_dir.NormalizeInPlace();
			//return final_dir;

			world_offset.NormalizeInPlace();
			return world_offset;
		}

		return point->direction;
	}

	Vector remix_lights::light::calculate_position_for_point(const map_settings::remix_light_settings_s::point_s* point) const
	{
		if (is_attached())
		{
#if 0
			Vector rotated_forward, rotated_right, rotated_up;
			utils::vector::AngleVectors(point->angle_offset_attached, &rotated_forward, &rotated_right, &rotated_up);

			rotated_forward = rotated_forward.x * m_attached_bone_forward + rotated_forward.y * m_attached_bone_right + rotated_forward.z * m_attached_bone_up;
			rotated_right = rotated_right.x * m_attached_bone_forward + rotated_right.y * m_attached_bone_right + rotated_right.z * m_attached_bone_up;
			rotated_up = rotated_up.x * m_attached_bone_forward + rotated_up.y * m_attached_bone_right + rotated_up.z * m_attached_bone_up;

			rotated_forward.NormalizeInPlace();
			rotated_right.NormalizeInPlace();
			rotated_up.NormalizeInPlace();

			Vector local_offset = point->position;
			Vector world_pos_offset = local_offset.x * rotated_forward + local_offset.y * rotated_right + local_offset.z * rotated_up;

			Vector final_pos = m_attached_position + world_pos_offset;
			return final_pos;
#endif

			// apply local position offset using bone's original basis
			const Vector world_pos_offset = point->position.x * m_attached_bone_forward + point->position.y * m_attached_bone_right + point->position.z * m_attached_bone_up;
			return m_attached_position + world_pos_offset;
		}

		return point->position;
	}

	/**
	 * Calculate light properties for the current tick \n
	 * All arguments besides 'parent' are optional - use nullptr to not update a specific property
	 * @param parent			(pointer to owning light)
	 * @param position			(remixapi_LightInfoSphereEXT)
	 * @param radiance			(remixapi_LightInfo)
	 * @param radius			(remixapi_LightInfo)
	 * @param direction			(remixapi_LightInfoSphereEXT)
	 * @param degrees			(remixapi_LightInfoSphereEXT)
	 * @param softness			(remixapi_LightInfoSphereEXT)
	 * @param exponent			(remixapi_LightInfoSphereEXT)
	 * @param volumetric_scale	(remixapi_LightInfoSphereEXT)
	 */
	void remix_lights::light::interpolator::interpolate(light* parent, remixapi_Float3D* position, remixapi_Float3D* radiance, float* radius,
		remixapi_Float3D* direction, float* degrees, float* softness, float* exponent, float* volumetric_scale, bool is_attached)
	{
		assert(parent != nullptr && "m_parent must not be null");
		if (!parent) {
			return;
		}

		{
			map_settings::remix_light_settings_s::point_s* temp_pt = nullptr;
			if (m_elapsed_time <= 0.0f) {
				temp_pt = &m_points.front();
			}
			else if (m_elapsed_time >= m_total_duration) {
				temp_pt = &m_points.back();
			}

			if (temp_pt)
			{
				if (position) { *position = parent->calculate_position_for_point(temp_pt).ToRemixFloat3D(); }//(temp_pt->position + m_parent->m_attached_position).ToRemixFloat3D(); }
				if (radiance) { *radiance = (temp_pt->radiance * temp_pt->radiance_scalar).ToRemixFloat3D(); }
				if (radius) { *radius = temp_pt->radius; }
				if (direction) { *direction = parent->calculate_direction_for_point(temp_pt).ToRemixFloat3D(); }
				if (degrees) { *degrees = temp_pt->degrees; }
				if (softness) { *softness = temp_pt->softness; }
				if (exponent) { *exponent = temp_pt->exponent; }
				if (volumetric_scale) { *volumetric_scale = temp_pt->volumetric_scale; }
				return;
			}
		}

		float time = m_elapsed_time;
		for (size_t i = 0; i < m_segment_durations.size(); ++i)
		{
			if (time <= m_segment_durations[i])
			{
				const auto t = time / m_segment_durations[i];
				const auto& p0 = (m_points)[((i - 1) + m_points.size()) % m_points.size()];
				const auto& p1 = (m_points)[i % m_points.size()];
				const auto& p2 = (m_points)[(m_loop_smoothing && i == m_points.size() - 1) ? 0 : (i + 1) % m_points.size()];
				const auto& p3 = (m_points)[(i + 2) % m_points.size()];

				const float t2 = t * t;
				const float t3 = t2 * t;

				// hermite interpolation
				if (position)
				{
					if (is_attached)
					{
						if (position)
						{
							Vector p0_world_offset = p0.position.x * parent->m_attached_bone_forward + p0.position.y * parent->m_attached_bone_right + p0.position.z * parent->m_attached_bone_up;
							Vector p1_world_offset = p1.position.x * parent->m_attached_bone_forward + p1.position.y * parent->m_attached_bone_right + p1.position.z * parent->m_attached_bone_up;
							Vector p2_world_offset = p2.position.x * parent->m_attached_bone_forward + p2.position.y * parent->m_attached_bone_right + p2.position.z * parent->m_attached_bone_up;
							Vector p3_world_offset = p3.position.x * parent->m_attached_bone_forward + p3.position.y * parent->m_attached_bone_right + p3.position.z * parent->m_attached_bone_up;

							Vector world_pos_offset =
								   p1_world_offset * ( 2.0f * t3 - 3.0f * t2 + 1.0f)
								+  p2_world_offset * (-2.0f * t3 + 3.0f * t2)
								+ (p2_world_offset - p0_world_offset) * p1.smoothness * (t3 - 2.0f * t2 + t)
								+ (p3_world_offset - p1_world_offset) * p2.smoothness * (t3 - t2);

							*position = (parent->m_attached_position + world_pos_offset).ToRemixFloat3D();


							//Vector finalPos = m_parent->m_attached_position + position_offset * final_dir;
							//*position = finalPos.ToRemixFloat3D();
						}
					}
					else
					{
						*position = (
							   p1.position * ( 2.0f * t3 - 3.0f * t2 + 1.0f)
							+  p2.position * (-2.0f * t3 + 3.0f * t2)
							+ (p2.position - p0.position) * p1.smoothness * (t3 - 2.0f * t2 + t)
							+ (p3.position - p1.position) * p2.smoothness * (t3 - t2)
							).ToRemixFloat3D();
					}
				}

				if (direction)
				{
					if (is_attached) 
					{
						// convert offsets to direction vectors
						Vector p0_offset, p1_offset, p2_offset, p3_offset;
						utils::vector::AngleVectors(p0.angle_offset_attached, &p0_offset);
						utils::vector::AngleVectors(p1.angle_offset_attached, &p1_offset);
						utils::vector::AngleVectors(p2.angle_offset_attached, &p2_offset);
						utils::vector::AngleVectors(p3.angle_offset_attached, &p3_offset);

						// transform offsets to bone's local coordinate system
						Vector p0_world_offset = p0_offset.x * parent->m_attached_bone_forward + p0_offset.y * parent->m_attached_bone_right + p0_offset.z * parent->m_attached_bone_up;
						Vector p1_world_offset = p1_offset.x * parent->m_attached_bone_forward + p1_offset.y * parent->m_attached_bone_right + p1_offset.z * parent->m_attached_bone_up;
						Vector p2_world_offset = p2_offset.x * parent->m_attached_bone_forward + p2_offset.y * parent->m_attached_bone_right + p2_offset.z * parent->m_attached_bone_up;
						Vector p3_world_offset = p3_offset.x * parent->m_attached_bone_forward + p3_offset.y * parent->m_attached_bone_right + p3_offset.z * parent->m_attached_bone_up;

						Vector interpolated_offset = (
							   p1_world_offset * ( 2.0f * t3 - 3.0f * t2 + 1.0f)
							+  p2_world_offset * (-2.0f * t3 + 3.0f * t2)
							+ (p2_world_offset - p0_world_offset) * p1.smoothness * (t3 - 2.0f * t2 + t)
							+ (p3_world_offset - p1_world_offset) * p2.smoothness * (t3 - t2)
							);

						// combine with bone's forward (scale to achieve full offset)
						float offset_scale = std::numbers::sqrt2_v<float>; // ca. 90deg max offset

						Vector final_dir = parent->m_attached_bone_forward + offset_scale * interpolated_offset;
							   final_dir.NormalizeInPlace();

						*direction = final_dir.ToRemixFloat3D();

						//if (position)
						//{

						//	// apply local position offset using bone's original basis
						//	const Vector world_pos_offset = point->position.x * m_attached_bone_forward + point->position.y * m_attached_bone_right + point->position.z * m_attached_bone_up;
						//	//return m_attached_position + world_pos_offset;


						//	// already up to date
						//	Vector position_offset { position->x, position->y, position->z };

						//	Vector finalPos = m_parent->m_attached_position + position_offset * final_dir;
						//	*position = finalPos.ToRemixFloat3D();
						//}
					}
					else
					{
						Vector dir = (
							   p1.direction * ( 2.0f * t3 - 3.0f * t2 + 1.0f)
							+  p2.direction * (-2.0f * t3 + 3.0f * t2)
							+ (p2.direction - p0.direction) * p1.smoothness * (t3 - 2.0f * t2 + t)
							+ (p3.direction - p1.direction) * p2.smoothness * (t3 - t2)
							);

						dir.Normalize();
						*direction = dir.ToRemixFloat3D();
					}
				}

				// linear interpolations

				if (radiance) {
					*radiance = lerp((p1.radiance * p1.radiance_scalar), (p2.radiance * p2.radiance_scalar), t).ToRemixFloat3D();
				}

				if (radius) {
					*radius = lerp(p1.radius, p2.radius, t);
				}

				if (degrees) {
					*degrees = lerp(p1.degrees, p2.degrees, t);
				}

				if (softness) {
					*softness = lerp(p1.softness, p2.softness, t);
				}

				if (exponent) {
					*exponent = lerp(p1.exponent, p2.exponent, t);
				}

				if (volumetric_scale) {
					*volumetric_scale = lerp(p1.volumetric_scale, p2.volumetric_scale, t);
				}

				return;
			}

			time -= m_segment_durations[i];
		}

		// should not happen if durations are correct
		const auto last_pt = m_points.back();
		if (position) { *position = parent->calculate_position_for_point(&last_pt).ToRemixFloat3D(); } //(last_pt.position + m_parent->m_attached_position).ToRemixFloat3D(); }
		if (radiance) { *radiance = (last_pt.radiance * last_pt.radiance_scalar).ToRemixFloat3D(); }
		if (radius) { *radius = last_pt.radius; }
		if (direction) { *direction = parent->calculate_direction_for_point(&last_pt).ToRemixFloat3D(); }
		if (degrees) { *degrees = last_pt.degrees; }
		if (softness) { *softness = last_pt.softness; }
		if (exponent) { *exponent = last_pt.exponent; }
		if (volumetric_scale) { *volumetric_scale = last_pt.volumetric_scale; }
	}

	// ----

	/**
	 * Update a remixApi light using an "external" point
	 * @param l			Light handle
	 * @param pt		External point handle
	 * @return			True if successfull
	 */
	bool remix_lights::update_static_remix_light(light* l, const map_settings::remix_light_settings_s::point_s* pt)
	{
		if (!l || !pt) {
			return false;
		}

		if (l->m_handle) {
			destroy_map_light(l);
		}

		if (l)
		{
			l->m_ext.position = l->calculate_position_for_point(pt).ToRemixFloat3D(); //(pt->position + l->m_attached_position).ToRemixFloat3D();
			l->m_info.radiance = (pt->radiance * pt->radiance_scalar).ToRemixFloat3D();
			l->m_ext.radius = pt->radius;
			l->m_ext.shaping_hasvalue = pt->use_shaping;
			l->m_ext.shaping_value.direction = l->calculate_direction_for_point(pt).ToRemixFloat3D();
			l->m_ext.shaping_value.coneAngleDegrees = pt->degrees;
			l->m_ext.shaping_value.coneSoftness = pt->softness;
			l->m_ext.shaping_value.focusExponent = pt->exponent;
			l->m_ext.volumetricRadianceScale = pt->volumetric_scale;

			// not updating these can result in a crash in bridge::remix_api?
			l->m_ext.pNext = nullptr;
			l->m_ext.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT;
			l->m_info.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO;
			l->m_info.pNext = &l->m_ext;

			return remix_api::get()->m_bridge.CreateLight(&l->m_info, &l->m_handle) == REMIXAPI_ERROR_CODE_SUCCESS;
		}

		return false;
	}

	/**
	 * Calculate and update remixApi light for the current tick
	 * @param l			The light
	 * @return			True if successfull
	 */
	bool remix_lights::update_remix_light(light* l)
	{
		if (!l || (l && !l->m_mover.is_initialized())) {
			return false;
		}

		if (l->m_handle) {
			destroy_map_light(l);
		}

		if (l)
		{
			l->m_mover.interpolate(
				l,
				&l->m_ext.position,
				&l->m_info.radiance,
				&l->m_ext.radius,
				&l->m_ext.shaping_value.direction,
				&l->m_ext.shaping_value.coneAngleDegrees,
				&l->m_ext.shaping_value.coneSoftness,
				&l->m_ext.shaping_value.focusExponent,
				&l->m_ext.volumetricRadianceScale, 
				l->is_attached());

			l->m_ext.shaping_hasvalue = l->m_ext.shaping_value.coneAngleDegrees != 180.0f;

			// not updating these can result in a crash in bridge::remix_api?
			l->m_ext.pNext = nullptr;
			l->m_ext.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT;
			l->m_info.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO;
			l->m_info.pNext = &l->m_ext;

			return remix_api::get()->m_bridge.CreateLight(&l->m_info, &l->m_handle) == REMIXAPI_ERROR_CODE_SUCCESS;
		}

		return false;
	}

	/**
	 * Spawns a remixApi light
	 * @param l			The light
	 * @return			True if successfull
	 */
	bool remix_lights::spawn_remix_light(light* l)
	{
		if (!l) {
			return false;
		}

		if (l->m_handle) {
			destroy_map_light(l);
		}

		if (l)
		{
			const auto& pt = l->m_def.points[0];
			l->m_ext.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT;
			l->m_ext.pNext = nullptr;
			l->m_ext.position = pt.position.ToRemixFloat3D();
			// disable single point lights with attach params until they get attached later down the line
			l->m_ext.radius = l->m_def.points.size() == 1u && l->has_attach_parms() ? 0.0f : pt.radius;
			l->m_ext.shaping_hasvalue = pt.use_shaping;
			l->m_ext.shaping_value = {};
			l->m_ext.shaping_value.direction = pt.direction.ToRemixFloat3D();
			l->m_ext.shaping_value.coneAngleDegrees = pt.degrees;
			l->m_ext.shaping_value.coneSoftness = pt.softness;
			l->m_ext.shaping_value.focusExponent = pt.exponent;
			l->m_ext.volumetricRadianceScale = pt.volumetric_scale;

			l->m_info.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO;
			l->m_info.pNext = &l->m_ext;
			l->m_info.hash = utils::string_hash64(utils::va("api-light%d", l->m_light_num));
			l->m_info.radiance = (pt.radiance * pt.radiance_scalar).ToRemixFloat3D();

			const auto api = remix_api::get();
			return api->m_bridge.CreateLight(&l->m_info, &l->m_handle) == REMIXAPI_ERROR_CODE_SUCCESS;
		}

		return false;
	}

	/**
	 * Adds all map_setting lights to 'm_map_lights' that have no defined trigger and removes them from the map_settings vector
	 */
	void remix_lights::add_all_map_setting_lights_without_creation_trigger()
	{
		// should have happend already, just to make sure
		get()->destroy_all_map_lights();

		auto& msettings = map_settings::get_map_settings();
		for (auto it = msettings.remix_lights.begin(); it != msettings.remix_lights.end();)
		{
			if (it->trigger_choreo_name.empty() && !it->trigger_sound_hash) // add lights without a trigger
			{
				m_active_lights.emplace_back(
					light {
						.m_def = *it, // do not move the light if it can be triggered multiple times
						.m_light_num = m_active_light_spawn_tracker++,
						.m_timer = it->kill_delay
					});

				// erase element from the mapsettings vector
				it = msettings.remix_lights.erase(it);

				auto* light = &m_active_lights.back();

				if (light->m_def.points.size() > 1) {
					light->m_mover.init(light->m_def.points, light->m_def.loop, light->m_def.loop_smoothing);
				}

				// spawn it
				get()->spawn_remix_light(light);
			}
			else { ++it; }
		}
	}


	void remix_lights::add_single_map_setting_light_for_editing(map_settings::remix_light_settings_s* def)
	{
		m_active_lights.emplace_back(
			light {
				.m_def = *def, // do not move the light if it can be triggered multiple times
				.m_light_num = m_active_light_spawn_tracker++
			});

		auto* light = &m_active_lights.back();

		if (light->m_def.points.size() > 1) {
			light->m_mover.init(light->m_def.points, true /* always loop*/, light->m_def.loop_smoothing);
		}

		// spawn it
		get()->spawn_remix_light(light);
	}

	/**
	 * Adds a single map setting light to 'm_map_lights' - Immediately spawns it if trigger is not defined
	 * @param def	The map_setting light definition
	 */
	void remix_lights::add_single_map_setting_light(map_settings::remix_light_settings_s* def)
	{
		m_active_lights.emplace_back(
			light {
				.m_def = *def, // do not move the light if it can be triggered multiple times
				.m_light_num = m_active_light_spawn_tracker++
			});

		// spawn light if it does not use a trigger - triggered spawning is handled elsewhere
		if (auto* light = &m_active_lights.back(); 
			light && light->m_def.trigger_choreo_name.empty() && !light->m_def.trigger_sound_hash)
		{
			if (light->m_def.points.size() > 1) {
				light->m_mover.init(light->m_def.points, light->m_def.loop, light->m_def.loop_smoothing);
			}

			// spawn it
			get()->spawn_remix_light(light);
		}
	}

	/**
	 * Destroys a light (remixApi light)
	 * @param l		The light to destroy
	 */
	void remix_lights::destroy_map_light(light* l)
	{
		if (l->m_handle)
		{
			remix_api::get()->m_bridge.DestroyLight(l->m_handle);
			l->m_handle = nullptr;
		}
	}

	/**
	 * Destroys all lights in 'm_map_lights' (remixApi lights)
	 */
	void remix_lights::destroy_all_map_lights()
	{
		for (auto& l : m_active_lights) {
			destroy_map_light(&l);
		}
	}

	/**
	 * Destroys all lights in 'm_map_lights' (remixApi lights) and clears 'm_map_lights'
	 */
	void remix_lights::destroy_and_clear_all_active_lights()
	{
		destroy_all_map_lights();
		m_active_lights.clear();
	}

	/**
	 * Updates all lights in 'm_map_lights'
	 * Handles Destroying, choreo trigger spawning, tick advancing and updating of remixApi lights
	 */
	void remix_lights::update_all_active_lights()
	{
		const auto glob = game::get_global_vars();
		const auto edit_mode = imgui::get()->m_light_edit_mode;

		// destroy lights that are marked for destruction
		for (auto it = m_active_lights.begin(); it != m_active_lights.end();)
		{
			if (it->m_is_marked_for_destruction)
			{
				// kill delay timer
				if (it->m_timer > 0.0f)
				{
					it->m_timer -= glob->absoluteframetime;
					++it;
				}
				else
				{
					destroy_map_light(&*it);
					it = m_active_lights.erase(it);
				}
			}
			else { ++it; }
		}

		// iterate all map lights
		for (auto& l : m_active_lights)
		{
			if (l.m_mover.is_initialized())
			{
				const auto finished = l.m_mover.advance_time(glob->absoluteframetime);
				update_remix_light(&l);

				if (!edit_mode)
				{
					if (finished && l.m_def.run_once) { // destroy light on next frame
						l.m_is_marked_for_destruction = true;
					}
				}
			}
			else // single point lights
			{
				if (l.has_attach_parms())
				{
					if (l.is_attached()) { // update every frame when attached
						update_static_remix_light(&l, &l.m_def.points.front());
					}
					else if (l.m_ext.radius > 0.0f) // "disable" light when it gets unattached
					{
						auto temp_pt = l.m_def.points.front();
						temp_pt.radius = 0.0f;
						update_static_remix_light(&l, &temp_pt);
					}
				}
			}

			// if light is not yet spawned
			if (!l.m_handle)
			{
				// handle delayed triggering
				if (l.m_timer < l.m_def.trigger_delay) {
					l.m_timer += glob->absoluteframetime;
				}
				else
				{
					if (l.m_def.points.size() > 1) {
						l.m_mover.init(l.m_def.points, l.m_def.loop, l.m_def.loop_smoothing);
					}

					// spawn it
					get()->spawn_remix_light(&l);

					// set timer to kill delay
					l.m_timer = l.m_def.kill_delay;
				}
			}
		}
	}

	// Draw all active map lights
	void remix_lights::draw_all_active_lights()
	{
		for (auto& l : m_active_lights)
		{
			if (l.m_handle) {
				remix_api::get()->m_bridge.DrawLightInstance(l.m_handle);
			}
		}
	}

	// #
	// #

	void remix_lights::a2_bts3_flashlight()
	{
		if (map_settings::is_level.sp_a2_bts3)
		{
			const auto remixapi = remix_api::get();

			if (!m_bts3_flashlight_pos.IsZero(0.0001f) && !m_bts3_wheatly_pos.IsZero(0.0001f))
			{
				auto dir = m_bts3_flashlight_pos - m_bts3_wheatly_pos;
				dir.Normalize();

				// offset pos along along direction so it does not start inside wheatly
				auto pos = m_bts3_wheatly_pos + dir * 20.0f;

				auto ext = remixapi_LightInfoSphereEXT
				{
					.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT,
					.pNext = nullptr,
					.position = remixapi_Float3D { pos.x, pos.y, pos.z },
					.radius = 1.2f,
					.shaping_hasvalue = TRUE,
					.shaping_value = {},
					.volumetricRadianceScale = 5.0f,
				};

				if (ext.shaping_hasvalue)
				{
					// ensure the direction is normalized
					ext.shaping_value.direction = { dir.x, dir.y, dir.z };
					ext.shaping_value.coneAngleDegrees = 45.0f;
					ext.shaping_value.coneSoftness = 0.2f;
					ext.shaping_value.focusExponent = 0;
				}

				const float lscale = 15.0f;

				auto info = remixapi_LightInfo
				{
					.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO,
					.pNext = &ext,
					.hash = utils::string_hash64("bts3fl"),
					.radiance = remixapi_Float3D { 150 * lscale, 140 * lscale, 130 * lscale },
				};

				if (m_bts3_flashlight_handle)
				{
					remixapi->m_bridge.DestroyLight(m_bts3_flashlight_handle);
					m_bts3_flashlight_handle = nullptr;
				}

				remixapi->m_bridge.CreateLight(&info, &m_bts3_flashlight_handle);

				if (m_bts3_flashlight_handle) {
					remixapi->m_bridge.DrawLightInstance(m_bts3_flashlight_handle);
				}

				// ---
				// sphere light at start of spotlight

				// bts3_flashlight_sphere_handle
				pos = m_bts3_wheatly_pos + dir * 9.0f;
				ext.position = { pos.x, pos.y, pos.z };
				ext.radius = 1.5f;
				ext.shaping_hasvalue = FALSE;
				ext.volumetricRadianceScale = 0.0f;

				info.hash = utils::string_hash64("bts3flsp");
				info.radiance = { info.radiance.x * 0.2f, info.radiance.y * 0.25f, info.radiance.z * 0.35f };

				if (m_bts3_flashlight_sphere_handle)
				{
					remixapi->m_bridge.DestroyLight(m_bts3_flashlight_sphere_handle);
					m_bts3_flashlight_sphere_handle = nullptr;
				}

				remixapi->m_bridge.CreateLight(&info, &m_bts3_flashlight_sphere_handle);

				if (m_bts3_flashlight_sphere_handle) {
					remixapi->m_bridge.DrawLightInstance(m_bts3_flashlight_sphere_handle);
				}

				// ---

				m_bts3_flashlight_pos = { 0, 0, 0 };
				m_bts3_wheatly_pos = { 0, 0, 0 };
			}
		}
	}

	void remix_lights::a2_bts3_flashlight_destroy()
	{
		const auto remixapi = remix_api::get();

		if (m_bts3_flashlight_handle)
		{
			remixapi->m_bridge.DestroyLight(m_bts3_flashlight_handle);
			m_bts3_flashlight_handle = nullptr;
		}

		if (m_bts3_flashlight_sphere_handle)
		{
			remixapi->m_bridge.DestroyLight(m_bts3_flashlight_sphere_handle);
			m_bts3_flashlight_sphere_handle = nullptr;
		}

		m_bts3_flashlight_pos = { 0, 0, 0 };
		m_bts3_wheatly_pos = { 0, 0, 0 };
	}

	// #
	// #

	bool light_attachment_is_matching_model(const remix_lights::light& light, const ModelRenderInfo_t& info)
	{
		const bool has_radius = light.m_def.attach_prop_radius != 0.0f;
		const bool has_name = !light.m_def.attach_prop_name.empty();

		if (!has_radius && !has_name) {
			return false;
		}

		// light is tracked via entity index if not -1
		if (light.m_entity_index >= 0) 
		{
			if (light.m_entity_index == info.entity_index)
			{
				// sanity check - making sure the entity is still valid
				if (has_radius && utils::float_equal(info.pModel->radius, light.m_def.attach_prop_radius)) {
					return true;
				}

				if (has_name && std::string_view(info.pModel->szPathName).contains(light.m_def.attach_prop_name)) {
					return true;
				}
			}

			return false;
		}

		// radius check if specified
		if (has_radius && !utils::float_equal(info.pModel->radius, light.m_def.attach_prop_radius)) {
			return false;
		}

		// bounds check
		if (!light.m_def.attach_prop_mins.IsZero() || !light.m_def.attach_prop_maxs.IsZero())
		{
			if (!utils::vector::is_point_in_aabb(info.origin, light.m_def.attach_prop_mins, light.m_def.attach_prop_maxs)) {
				return false;
			}
		}

		// name substring check if specified
		if (has_name && !std::string_view(info.pModel->szPathName).contains(light.m_def.attach_prop_name)) {
			return false;
		}

		return true;
	}


	// called from model_renderer::DrawModelExecute::Detour
	void remix_lights::on_draw_model_exec(const ModelRenderInfo_t& info)
	{
		if (cmd::show_mesh_bone_info)
		{
			const auto cutoff_dist = game_settings::get()->debug_info_distance.get_as<float>();
			if (g_player_view_org.DistToSqr(info.origin) < cutoff_dist * cutoff_dist)
			{
				if (const auto base_animating = game::get_base_animating_for_client_renderable(info.pRenderable);
					base_animating)
				{
					if (const auto studio = game::namespaces::C_BaseAnimating::GetModelPtr(base_animating);
						studio)
					{
						auto studio_mdl = studio->m_pStudioHdr;
						Vector bonePos;
						Vector boneAngles;
						matrix3x4_t bone = {};

						// vis all bones
						for (int i = 0; i < studio_mdl->numbones; i++)
						{
							game::namespaces::C_BaseAnimating::GetBoneTransform(base_animating, i, &bone);

							// C_BaseAnimating::GetBonePosition
							utils::matrix_angles(bone, &boneAngles);
							bonePos.x = bone.m_flMatVal[0][3];
							bonePos.y = bone.m_flMatVal[1][3];
							bonePos.z = bone.m_flMatVal[2][3];

							const auto pBone = studio_mdl->pBone(i);
							const auto bname = pBone->pszName();
							game::debug_add_text_overlay(&bonePos.x, utils::va("Bone: '%d' -- '%s'", i, bname), 0);
							game::debug_add_text_overlay(&bonePos.x, utils::va("Ent Index: '%d'", info.entity_index), 1);
						}
					}
				}
			}
		}


		if (map_settings::get_map_settings().using_any_light_attached_to_prop || imgui::get()->m_light_edit_mode)
		{
			for (auto& light : m_active_lights)
			{
				if (light.has_attach_parms() && light.m_attachframe != m_attachframe_counter)
				{
					if (light_attachment_is_matching_model(light, info))
					{
						light.m_attachframe = m_attachframe_counter; // mark as processed this frame
						light.m_attached_position = info.origin;
						light.m_attached_angle = info.angles;
						light.m_entity_index = info.entity_index; // track entity_index so that the light stays attached even when the ent moves outside

						// logic if light should attach to a bone
						if (cmd::show_mesh_bone_info_attached || light.m_def.attach_bone_index >= 0 || !light.m_def.attach_bone_name.empty()) 
						{
							if (const auto base_animating = game::get_base_animating_for_client_renderable(info.pRenderable);
								base_animating)
							{
								if (const auto studio = game::namespaces::C_BaseAnimating::GetModelPtr(base_animating);
									studio)
								{
									matrix3x4_t bone = {};
									Vector bonePos;

									// do not redraw info if showing info for every mesh
									if (!cmd::show_mesh_bone_info && cmd::show_mesh_bone_info_attached)
									{
										auto studio_mdl = studio->m_pStudioHdr;
										Vector boneAngles;

										// vis all bones
										for (int i = 0; i < studio_mdl->numbones; i++)
										{
											game::namespaces::C_BaseAnimating::GetBoneTransform(base_animating, i, &bone);

											// C_BaseAnimating::GetBonePosition
											utils::matrix_angles(bone, &boneAngles);
											bonePos.x = bone.m_flMatVal[0][3];
											bonePos.y = bone.m_flMatVal[1][3];
											bonePos.z = bone.m_flMatVal[2][3];

											const auto pBone = studio_mdl->pBone(i);
											const auto bname = pBone->pszName();
											game::debug_add_text_overlay(&bonePos.x, utils::va("Bone: '%d' -- '%s'", i, bname), 0);
											game::debug_add_text_overlay(&bonePos.x, utils::va("Ent Index: '%d'", info.entity_index), 1);
										}
									}

									int bone_idx = light.m_def.attach_bone_index;
									if (bone_idx < 0 && !light.m_def.attach_bone_name.empty()) {
										bone_idx = game::namespaces::C_BaseAnimating::LookupBone(base_animating, light.m_def.attach_bone_name.c_str());
									}

									if (bone_idx >= 0)
									{
										game::namespaces::C_BaseAnimating::GetBoneTransform(base_animating, bone_idx, &bone);

										light.m_attached_bone_forward = {
											bone.m_flMatVal[0][0], bone.m_flMatVal[1][0], bone.m_flMatVal[2][0]
										};

										light.m_attached_bone_right = {
											bone.m_flMatVal[0][1], bone.m_flMatVal[1][1], bone.m_flMatVal[2][1]
										};

										light.m_attached_bone_up = {
											bone.m_flMatVal[0][2], bone.m_flMatVal[1][2], bone.m_flMatVal[2][2]
										};

										// GetBonePosition(i, bonePos, boneAngles);
										utils::matrix_angles(bone, &light.m_attached_angle);
										bonePos.x = bone.m_flMatVal[0][3];
										bonePos.y = bone.m_flMatVal[1][3];
										bonePos.z = bone.m_flMatVal[2][3];

										light.m_attached_position = bonePos;
									}
								}
							}
						}
					}
				}

				// reset tracked entity when the entity could not be found for 5 frames
				if (!light.is_attached() && m_attachframe_counter > light.m_attachframe + 5) {
					light.m_entity_index = -1;
				}
			}
		}
	}

	// called from: choreo_events::scene_ent_on_start_event_hk
	void remix_lights::on_event_start(const std::string_view& name, const std::string_view& actor, const std::string_view& event, const std::string_view& param1)
	{
		// no event trigger in edit mode
		if (imgui::get()->m_light_edit_mode) {
			return;
		}

		auto& msettings = map_settings::get_map_settings();
		for (auto it = msettings.remix_lights.begin(); it != msettings.remix_lights.end();)
		{
			if (!it->trigger_choreo_name.empty() && name.contains(it->trigger_choreo_name))
			{
				// check if opt. actor is defined and matches event actor
				if (!it->trigger_choreo_actor.empty() && !actor.contains(it->trigger_choreo_actor)) {
					++it; continue;
				}

				// check if opt. event is defined and matches event string
				if (!it->trigger_choreo_event.empty() && !event.contains(it->trigger_choreo_event)) {
					++it; continue;
				}

				// check if opt. param1 is defined and matches event param1
				if (!it->trigger_choreo_param1.empty() && !param1.contains(it->trigger_choreo_param1)) {
					++it; continue;
				}

				get()->add_single_map_setting_light(&*it);

				// only spawn on the very first play of the vcd
				if (!it->trigger_always) {
					it = msettings.remix_lights.erase(it); // erase element from the mapsettings vector
				}
				else { ++it; }
			}
			else { ++it; }
		}
	}

	// called from: choreo_events::scene_ent_on_finish_event_hk
	void remix_lights::on_event_finish(const std::string_view& name)
	{
		// no event trigger in edit mode
		if (imgui::get()->m_light_edit_mode) {
			return;
		}

		for (auto& l : m_active_lights)
		{
			// only check active lights with a kill trigger not yet marked to be destroyed
			if (l.m_handle && !l.m_def.kill_choreo_name.empty() && !l.m_is_marked_for_destruction)
			{
				if (name.contains(l.m_def.kill_choreo_name)) {
					l.m_is_marked_for_destruction = true;
				}
			}
		}
	}

	void remix_lights::on_sound_start(const std::uint32_t hash)
	{
		// no event trigger in edit mode
		if (imgui::get()->m_light_edit_mode) {
			return;
		}

		// check for kill trigger
		for (auto& l : m_active_lights)
		{
			// only check active lights with a kill trigger not yet marked to be destroyed
			if (l.m_handle && l.m_def.kill_sound_hash && !l.m_is_marked_for_destruction)
			{
				if (l.m_def.kill_sound_hash == hash) {
					l.m_is_marked_for_destruction = true;
				}
			}
		}

		// check for spawn trigger
		auto& msettings = map_settings::get_map_settings();
		for (auto it = msettings.remix_lights.begin(); it != msettings.remix_lights.end();)
		{
			if (it->trigger_sound_hash == hash)
			{
				get()->add_single_map_setting_light(&*it);

				// only spawn on the very first play of sound
				if (!it->trigger_always) {
					it = msettings.remix_lights.erase(it); // erase element from the mapsettings vector
				}
				else { ++it; }
			}
			else { ++it; }
		}
	}

	void remix_lights::on_client_frame()
	{
		const auto rml = remix_lights::get();
		const auto& glob = game::get_global_vars();

		// check if paused
		rml->m_is_paused = utils::float_equal(glob->frametime, 0.0f);
		
		if (!rml->m_is_paused)
		{
			rml->update_all_active_lights();
 			++m_attachframe_counter;
		}

		// hardcoded wheatly flashlight
		if (game_settings::get()->use_hardcoded_wheatly_flashlight_bts3.get_as<bool>()) {
			rml->a2_bts3_flashlight();
		}

		rml->draw_all_active_lights();

		if (cmd::show_api_lights)
		{
			bool first_done = false;
			for (const auto& l : m_active_lights)
			{
				const Vector circle_pos = &l.m_ext.position.x;
				const float radius = l.m_ext.radius;
				const Vector color = { 1.0f, 1.0f, 1.0f };

				const auto remixapi = remix_api::get();

				// we only need to craft one circle instance - everything else is instanced
				if (!first_done)
				{
					first_done = true;
					remixapi->add_debug_circle(circle_pos, Vector(0.0f, 0.0f, 1.0f), radius - 0.02f, radius * 0.1f, color);
				}
				else {
					remixapi->add_debug_circle_based_on_previous(circle_pos, Vector(0, 0, 90), Vector(1.0f, 1.0f, 1.0f));
				}

				remixapi->add_debug_circle_based_on_previous(circle_pos, Vector(0, 90, 0), Vector(1.0f, 1.0f, 1.0f));
				remixapi->add_debug_circle_based_on_previous(circle_pos, Vector(90, 0, 90), Vector(1.0f, 1.0f, 1.0f));
			}
		}

	}

	// called before map_settings
	void remix_lights::on_map_load()
	{
		if (game_settings::get()->use_hardcoded_wheatly_flashlight_bts3.get_as<bool>()) {
			get()->a2_bts3_flashlight_destroy();
		}

		// reset spawn tracker
		m_active_light_spawn_tracker = 0u;
		m_attachframe_counter = 0u;
	}

	ConCommand xo_debug_toggle_show_api_lights_cmd{};
	void xo_debug_toggle_show_api_lights_fn()
	{
		cmd::show_api_lights = !cmd::show_api_lights;
	}

	ConCommand xo_debug_show_mesh_bone_info_attached_cmd{};
	void xo_debug_show_mesh_bone_info_attached_fn()
	{
		cmd::show_mesh_bone_info_attached = !cmd::show_mesh_bone_info_attached;

		// disable vis for ALL 
		if (cmd::show_mesh_bone_info_attached) {
			cmd::show_mesh_bone_info = false;
		}
	}

	ConCommand xo_debug_show_mesh_bone_info_cmd{};
	void xo_debug_show_mesh_bone_info_fn()
	{
		cmd::show_mesh_bone_info = !cmd::show_mesh_bone_info;

		// disable specific vis if still active
		if (!cmd::show_mesh_bone_info) {
			cmd::show_mesh_bone_info_attached = false;
		}
	}

	remix_lights::remix_lights()
	{
		p_this = this;

		// #
		// commands

		game::con_add_command(&xo_debug_toggle_show_api_lights_cmd, "xo_debug_toggle_show_api_lights", xo_debug_toggle_show_api_lights_fn, "Toggle debug vis for lights added via the remixapi");
		game::con_add_command(&xo_debug_show_mesh_bone_info_attached_cmd, "xo_debug_show_mesh_bone_info_attached", xo_debug_show_mesh_bone_info_attached_fn, "Edit Mode + Attached to mesh only: Show bone information of mesh with an attached remixApi light (names/indices)");
		game::con_add_command(&xo_debug_show_mesh_bone_info_cmd, "xo_debug_show_mesh_bone_info", xo_debug_show_mesh_bone_info_fn, "Show bone information for all nearby meshes (names/indices + entity indices)");
	}
}
